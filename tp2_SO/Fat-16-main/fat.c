#include "tp.h"

int init()
{
	int i = 0;
	FILE *arq = fopen("fat.part", "wb");
	if (arq == NULL)
	{
		printf("ERRO AO CRIAR DISCO FAT\n");
		return 0;
	}
	//boot_block
	uint8_t boot_block = 0xbb;
	for (i; i < CLUSTER_SIZE; i++)
	{
		fwrite(&boot_block, sizeof(boot_block), 1, arq);
	}
	//preencher tabela fat
	fat[0] = 0xfffd;
	i = 1;
	for (i; i < 10; i++)
	{
		fat[i] = 0xfffe;
	}
	fat[9] = 0xffff;
	for (i; i < 4096; i++)
	{
		fat[i] = 0x00;
	}
	//FAT
	fwrite(fat, sizeof(uint16_t), 4096, arq); //salva o fat no arquivo de 4096 entradas de 16 bits
	//Root dir
	dir_entry_t root_dir[32];
	memset(root_dir, 0x00, sizeof(root_dir));				//1 cluster
	fwrite(root_dir, sizeof(dir_entry_t), 32, arq); //salva o root_dir no arquivo (32 entradas de diretório)
	//Data Cluesters
	uint8_t data[CLUSTER_SIZE];
	memset(data, 0x00, CLUSTER_SIZE); //cluster
	i = 0;
	for (i; i < 4086; i++)
	{																			// 4086 clusters
		fwrite(data, 1, CLUSTER_SIZE, arq); //salvar cluster no arquivo fat.part
	}
	fclose(arq);
	return 1;
}

int load()
{
	FILE *arq = fopen("fat.part", "rb"); //abre a partição fat
	if (arq == NULL)
	{
		printf("ERRO NA LEITURA DE DISCO FAT\n");
		return 0;
	}
	fseek(arq, CLUSTER_SIZE, SEEK_SET);			 //Aponta para o FAT após o boot_block de 1024 bytes
	fread(fat, sizeof(uint16_t), 4096, arq); // Ler o FAT com 4096 entradas de 16 bits
	//fread(root_dir, sizeof(dir_entry_t), 32, arq); // Ler diretorio raiz com 32 entradas
	fclose(arq);
	return 1;
}

void ls(char *diretorio)
{
	if (diretorio == NULL)
	{
		printf("CAMINHO INVALIDO\n");
		return;
	}
	char *dirAtual = (char *)malloc(sizeof(char) * STRINGS_SIZE); // Receberá o nome do diretorio no caminho
	int index;
	index = procurarDIr(diretorio, dirAtual, 2); //Recebe o index do caminho, -1 se não for encontrado
	if (index != -1)
	{ //verifica se encontrou o arquivo
		data_cluster data = lerCluster(index);
		printf("DIRETORIOS:\n");
		for (int i = 0; i < 32; i++)
		{ //Percorre as 32 entradas e verifica se é um diretorio
			if (data.dir[i].first_block != 0 && data.dir[i].attributes == 1)
			{ //se for um diretório printa o filename
				printf("%s ", data.dir[i].filename);
			}
		}
		printf("\nARQUIVOS:\n");
		for (int i = 0; i < 32; i++)
		{ //Percorre as 32 entradas e verifica se é um arquivo
			if (data.dir[i].first_block != 0 && data.dir[i].attributes == 0)
			{ //se for um arquivo printa o filename
				printf("%s ", data.dir[i].filename);
			}
		}
		printf("\n");
	}
	free(dirAtual); //desaloca
}

void mkdir(char *diretorio)
{
	if (diretorio == NULL || strcmp(diretorio, "") == 0)
	{
		printf("Caminho invalido\n");
		return;
	}
	if (strcmp(diretorio, "/") == 0)
	{
		printf("NAO E POSSIVEL CRIAR O DIRETORIO RAIZ\n");
		return;
	}
	char *dirAtual = (char *)malloc(sizeof(char) * STRINGS_SIZE); // Recebera o nome do diretorio no caminho
	int index;
	index = procurarDIr(diretorio, dirAtual, 1); //procura o index do cluster do diretorio pai do diretório a ser criado
	if (index != -1)
	{ //verifica se encontrou um diretório pai
		if (strcmp(dirAtual, "") != 0 && strcmp(dirAtual, "/") != 0 && strcmp(dirAtual, " ") != 0)
		{ //caso o parametro do comando não seja vazio ou somente "/" (porque nesse caso o usuário estaria tentando criar um diretorio sem nome), o processo prossegue
			data_cluster data = lerCluster(index);
			data_cluster novoDiretorio; //cluster que recebrá as entradas do novo diretório
			int indexBloco, j;
			memset(novoDiretorio.dir, 0x00, 32 * sizeof(dir_entry_t)); //preeche o cluster  com 32 entradas vazias
			for (j = 0; j < 32; j++)
			{ //percorre as 32 entradas no diretorio pai
				if (strcmp(data.dir[j].filename, dirAtual) == 0 && data.dir[j].attributes == 1)
				{ //garante se ja existe diretorio com este nome
					printf("DIRETORIO JA EXISTE\n");
					free(dirAtual);
					return;
				}
				if (data.dir[j].first_block == 0)
				{ //se a posição estiver vazia para o loop
					break;
				}
			}
			if (j == 32)
			{ //se j for 32 todas as entradas do diretório estão preenchidas
				printf("DIRETORIO CHEIO\n");
				free(dirAtual);
				return;
			}
			if (strlen(dirAtual) < 18)
			{																														 //se o nome do direório for maior que o limite 18 caracters é tratado no else
				strncpy(data.dir[j].filename, dirAtual, strlen(dirAtual)); //se for de tamanho correto, é salvo o nome
			}
			else
			{ //limitamos a copia para 17 caracters, o 18 sendo o \0
				strncpy(data.dir[j].filename, dirAtual, 17);
			}
			data.dir[j].attributes = 1; //1 - diretorio
			for (indexBloco = 10; indexBloco < 4096; indexBloco++)
			{ //percorre a fat
				if (fat[indexBloco] == 0x00)
				{ //procura uma posição vazia
					data.dir[j].first_block = indexBloco;
					fat[indexBloco] = 0xffff;
					atualizarFat();														//atualiza a fat no arquivo
					salvarCluster(index, data);								//atualiza o cluster do direório pai
					salvarCluster(indexBloco, novoDiretorio); // salva o novo diretorio no arquivo
					free(dirAtual);
					return;
				}
			}
		}
		else
		{
			printf("PARAMETRO INVALIDO\n");
		}
	}
	free(dirAtual);
}

void create(char *diretorio)
{
	if (diretorio == NULL || strcmp(diretorio, "") == 0)
	{
		printf("Caminho invalido\n");
		return;
	}
	char *dirAtual = (char *)malloc(sizeof(char) * STRINGS_SIZE); // Receberá o nome do arquivo no caminho
	int index;
	index = procurarDIr(diretorio, dirAtual, 1); //procura o index do cluster do diretório onde o arquivo será salvo
	if (index != -1)
	{ //verifica se encontrou o diretório
		if (strcmp(dirAtual, "") != 0 && strcmp(dirAtual, "/") != 0 && strcmp(dirAtual, " ") != 0)
		{ //caso o parametro do comando não seja vazio ou somente "/" (porque nesse caso o usuário estaria tentando criar um arquivo sem nome), o processo prossegue
			data_cluster data = lerCluster(index);
			data_cluster novoArquivo; //cluster que receberá o espaço do novo arquivo
			int indexBloco, j;
			memset(novoArquivo.data, 0x00, CLUSTER_SIZE); //preecnhe o cluster do novo arquivo
			for (j = 0; j < 32; j++)
			{ //percorre o diretório onde o arquivo será salvo
				if (strcmp(data.dir[j].filename, dirAtual) == 0 && data.dir[j].attributes == 0)
				{ //verifica se ja existe arquivo com este nome
					printf("ARQUIVO JA EXISTE\n");
					free(dirAtual);
					return;
				}
				if (data.dir[j].first_block == 0)
				{ //se encontrar um espço vazio para o loop
					break;
				}
			}

			if (j == 32)
			{ //se perorrer todas as entradas significa que o diretório está cheio
				printf("DIRETORIO CHEIO\n");
				free(dirAtual);
				return;
			}

			if (strlen(dirAtual) < 18)
			{																														 //se o nome do arquivo for maior que o limite 18 caracters é tratado no else
				strncpy(data.dir[j].filename, dirAtual, strlen(dirAtual)); //se for de tamanho correto, é salvo o nome
			}
			else
			{ //limitamos a copia para 17 caracters, o 18 sendo o \0
				strncpy(data.dir[j].filename, dirAtual, 17);
			}
			data.dir[j].attributes = 0; //0 - arquivo
			for (indexBloco = 10; indexBloco < 4096; indexBloco++)
			{ //percorre a fat
				if (fat[indexBloco] == 0x00)
				{ //procura uma posição vazia na fat
					data.dir[j].first_block = indexBloco;
					fat[indexBloco] = 0xffff;
					atualizarFat();													//atualiza a fat no arquivo
					salvarCluster(index, data);							//atualiza o cluster
					salvarCluster(indexBloco, novoArquivo); // salva no arquivo
					free(dirAtual);
					return;
				}
			}
		}
		else
		{
			printf("PARAMETRO INVALIDO\n");
		}
	}
	free(dirAtual);
}

void unlink(char *diretorio)
{
	if (diretorio == NULL || strcmp(diretorio, "") == 0)
	{
		printf("CAMINHO INVALIDO\n");
		return;
	}
	if (strcmp(diretorio, "/") == 0)
	{
		printf("NAO E POSSIVEL EXCLUIR O DIRETORIO RAIZ\n");
		return;
	}
	char *dirAtual = (char *)malloc(sizeof(char) * STRINGS_SIZE); // Receberá o nome do diretorio ou arquivo no caminho que devera ser apagado
	int index;
	index = procurarDIr(diretorio, dirAtual, 1); //procura o index do diretorio pai do diretório que será salvo, e no caso de arquivo será o diretório onde o arquivo se encontra
	if (index != -1)
	{
		if (strcmp(dirAtual, "") != 0 && strcmp(dirAtual, "/") != 0 && strcmp(dirAtual, " ") != 0)
		{ //caso o parametro do comando não seja vazio ou somente "/" (porque nesse caso o usuário estaria tentando apagar um diretorio ou arquivo sem nome), o processo prossegue
			data_cluster aux;
			data_cluster data = lerCluster(index); // ler cluster pai do que deve ser apagado
			dir_entry_t dirVazio;									 //diretorio vazio
			int j, i;
			memset(&dirVazio, 0x00, 32); //preecnhe o diretorio
			for (i = 0; i < 32; i++)
			{ //percorre todas as entradas do diretorio do index
				if (data.dir[i].first_block != 0 && strcmp(dirAtual, data.dir[i].filename) == 0)
				{ //se econtrar um espaço que não esteja vazio e seja o arquivo ou diretorio que será apagado
					if (data.dir[i].attributes == 1)
					{																						 //verifica se é um diretório
						aux = lerCluster(data.dir[i].first_block); //ler o cluster do diretório que será excluido
						for (j = 0; j < 32; j++)
						{ //percorre o diretorio que sera excluido
							if (aux.dir[j].first_block != 0)
							{ //se ele tiver uma posição preenchida para o loop
								break;
							}
						}
						if (j == 32)
						{																			 //se percorrer todo direório significa que está vazio
							fat[data.dir[i].first_block] = 0x00; //vazio
							data.dir[i] = dirVazio;							 //recebe diretório vazio
							salvarCluster(index, data);					 //salva o diretório pai
							atualizarFat();											 //atualiza a fat
							printf("DIRETORIO APAGADO\n");
							free(dirAtual);
							return;
						}
						else
						{ //se o j for menor que 32 significa que exite algum arquivo ou diretorio dentro do diretorio que deseja apagar
							printf("ESVAZIE O DIRETORIO ANTES DE FAZER UNLINK\n");
							free(dirAtual);
							return;
						}
					}
					if (data.dir[i].attributes == 0)
					{ //verifica se é um árquivo
						int indexaux = data.dir[i].first_block;
						//indexaux = fat[indexaux];
						while (fat[indexaux] != 0xffff)
						{
							j = indexaux;
							indexaux = fat[indexaux];
							fat[j] = 0;
						}
						//data.dir[i].first_block
						fat[data.dir[i].first_block] = 0x00; //limpa o ultimo da fat
						fat[indexaux] = 0x00;								 //vazio
						data.dir[i] = dirVazio;							 //recebe o diretorio vazio
						salvarCluster(index, data);					 //salva o cluster do diretorio onde se encontra o arquivo
						atualizarFat();											 //atualiza a fat
						printf("ARQUIVO APAGADO\n");
						free(dirAtual);
						return;
					}
					printf("ERRO DE ATTRIBUTES\n");
				}
			}
			if (i == 32)
			{
				printf("DIRETORIO NAO ENCONTRADO\n");
			}
		}
		else
		{
			printf("PARAMETRO INVALIDO\n");
		}
	}
	free(dirAtual);
}

void write(char *parametros)
{
	if (parametros == NULL || strcmp(parametros, "") == 0)
	{
		printf("CAMINHO INVALIDO\n");
		return;
	}
	//strings que iram receber o diretorio e string e o nome do arquivo
	char *diretorio = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	char *string = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	char *dirAtual = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	separaString(parametros, string, diretorio, "/");
	if (strcmp(string, "") == 0)
	{
		printf("STRING VAZIA\n");
		free(diretorio);
		free(string);
		free(dirAtual);
		return;
	}
	int index;
	index = procurarDIr(diretorio, dirAtual, 3); //procura o cluster do diretorio onde o arquivo
	if (index != -1)
	{
		data_cluster data = lerCluster(index);
		int i;
		for (i = 0; i < 32; i++)
		{ //verifica as 32 entradas diretorio
			if (strcmp(data.dir[i].filename, dirAtual) == 0 && data.dir[i].first_block != 0 && data.dir[i].attributes == 0)
			{ //para o loop se achar o arquivo
				index = data.dir[i].first_block;
				break;
			}
		}
		if (i == 32)
		{ //se i for 32 o arquivo não foi econtrado
			printf("ARQUIVO NAO ENCONTRADO NO DIRETORIO\n");
			free(diretorio);
			free(string);
			free(dirAtual);
			return;
		}
		int numClusters;
		data_cluster *clusters;
		clusters = quebrarStringClusters(string, &numClusters); // separa a string em clusters
		int auxindex = index, indexBloco;
		while (fat[auxindex] != 0xffff)
		{ //procura espaço vazio
			indexBloco = auxindex;
			auxindex = fat[auxindex];
			fat[indexBloco] = 0x00;
		}
		fat[auxindex] = 0x00;
		fat[index] = 0xffff;
		salvarCluster(index, clusters[0]); //salva o primeiro cluster
		if (numClusters > 1)
		{ //se tiver mais algum cluster salva eles
			for (i = 1; i < numClusters; i++)
			{
				for (indexBloco = 10; indexBloco < 4096; indexBloco++)
				{ //procura espaço vazio
					if (fat[indexBloco] == 0x00)
					{
						break;
					}
				}
				fat[index] = indexBloco;
				index = indexBloco;
				fat[index] = 0xffff;							 // salva na fat
				salvarCluster(index, clusters[i]); //salva o cluster i no arquivo
			}
		}
		atualizarFat(); //atualiza a fat
	}
	else
	{
		printf("ARQUIVO NAO ENCONTRADO\n");
	}
	free(diretorio);
	free(string);
	free(dirAtual);
}

void append(char *parametros)
{
	if (parametros == NULL || strcmp(parametros, "") == 0)
	{
		printf("CAMINHO INVALIDO\n");
		return;
	}
	//strings que receberão os pedaços do parametro
	char *diretorio = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	char *string = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	char *dirAtual = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	separaString(parametros, string, diretorio, "/"); //separa a string que será salva do diretório
	if (strcmp(string, "") == 0)
	{ //veirifca a string
		printf("STRING VAZIA\n");
		free(diretorio);
		free(string);
		free(dirAtual);
		return;
	}
	int index;
	index = procurarDIr(diretorio, dirAtual, 3); //procura o cluster do diretório onde o arquivo está, o qual será salvo a string
	if (index != -1)
	{																				 //verifica se encontrou o cluster do diretorio onde arquivo
		data_cluster data = lerCluster(index); //ler o cluster do diretório onde o arquivo está
		int i, tamArq;
		for (i = 0; i < 32; i++)
		{ //percorre o cluster
			if (strcmp(data.dir[i].filename, dirAtual) == 0 && data.dir[i].first_block != 0 && data.dir[i].attributes == 0)
			{ //se achar o arquivo para o loop
				index = data.dir[i].first_block;
				break;
			}
		}
		if (i == 32)
		{ //não achou o arquivo
			printf("ARQUIVO NAO ENCONTRADO NO DIRETORIO\n");
			free(diretorio);
			free(string);
			free(dirAtual);
			return;
		}
		while (fat[index] != 0xffff)
		{ //procura o ultimo cluster que pertence ao arquivo
			index = fat[index];
		}
		data = lerCluster(index);		//ler cluster
		tamArq = strlen(data.data); //pega o tamanho que já está ocupado
		if (1024 > tamArq + strlen(string))
		{														 //se a streing cabe no cluster
			strcat(data.data, string); //coloca a string junto o que já tem
			salvarCluster(index, data);
		}
		else
		{
			data_cluster *clusters; //cria uma lista de clusters
			int numClusters = 0, indexBloco, tam;
			tam = 1023 - tamArq;
			strncat(data.data, string, tam);																				//concatena com os n caracters da string
			salvarCluster(index, data);																							//salva o que da no cluster atual
			clusters = quebrarStringClusters(&string[1024 - tamArq], &numClusters); //preenche a lista com os clusters necessarios
			for (indexBloco = 10; indexBloco < 4096; indexBloco++)
			{ //percorre a fat
				if (fat[indexBloco] == 0x00)
				{ //procura espaço vazio na fat
					break;
				}
			}
			fat[index] = indexBloco;
			fat[indexBloco] = 0xffff;
			salvarCluster(indexBloco, clusters[0]); //salva na fat o primeiro cluster
			if (numClusters > 1)
			{
				for (i = 1; i < numClusters; i++)
				{ //salva os clusters restantes se necessario
					int indexBloco;
					for (indexBloco = 10; indexBloco < 4096; indexBloco++)
					{ //percorre a fat
						if (fat[indexBloco] == 0x00)
						{ //para o loop se encontrar espeço vazio
							break;
						}
					}
					fat[index] = indexBloco;
					index = indexBloco;
					fat[index] = 0xffff;
					salvarCluster(index, clusters[i]);
				}
			}
			atualizarFat(); //atualiza a fat
		}
	}
	else
	{
		printf("ARQUIVO NAO ENCONTRADO\n");
	}
	free(diretorio);
	free(string);
	free(dirAtual);
}

void read(char *diretorio)
{
	if (diretorio == NULL || strcmp(diretorio, "") == 0 || strcmp(diretorio, "/") == 0)
	{
		printf("CAMINHO INVALIDO\n");
		return;
	}
	char *dirAtual = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	int index;
	index = procurarDIr(diretorio, dirAtual, 3); //procura o arquivo
	if (index != -1)
	{																				 //verifica se arquivo foi encontrado
		data_cluster data = lerCluster(index); //ler o cluster o diretorio
		int i, tamArq;
		for (i = 0; i < 32; i++)
		{ //verifica as entradas
			if (strcmp(data.dir[i].filename, dirAtual) == 0 && data.dir[i].first_block != 0 && data.dir[i].attributes == 0)
			{ //para se encontrar o arquivo
				index = data.dir[i].first_block;
				break;
			}
		}
		if (i == 32)
		{ //caso tenha ocorrido algum erro
			printf("ARQUIVO NAO ENCONTRADO NO DIRETORIO\n");
			free(dirAtual);
			return;
		}
		data = lerCluster(index);
		uint8_t arquivo[1024];										//tamanho do arquivo
		snprintf(arquivo, 1024, "%s", data.data); //printa o primeiro cluster do arquivo
		printf("%s", arquivo);
		while (fat[index] != 0xffff)
		{ //procura e printa os outros clusters
			index = fat[index];
			data = lerCluster(index);
			snprintf(arquivo, 1024, "%s", data.data); //printa 1024 caracters,bits
			printf("%s", arquivo);
		}
	}
	else
	{
		printf("ARQUIVO NAO ENCONTRADO");
	}
	free(dirAtual);
	printf("\n");
}