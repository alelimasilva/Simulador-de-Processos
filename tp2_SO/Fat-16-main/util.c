#include "tp.h"
//diretorios
int procurarDIr(char *diretorio, char *dirAtual, int procura)
{ //procura index do cluster do caminho recebido
	int index = 9, j = 0, k = 0;
	int indexPai = index;
	if (strcmp(diretorio, "/") == 0)
	{								//diretório raiz
		return index; //index
	}

	if (diretorio[0] != '/')
	{ //caminho invalido
		printf("PARAMETRO INVALIDO\n");
		return -1;
	}
	int numDiretorios = getNumDiretorios(diretorio); //numero de caminhos após o diretório raiz
	data_cluster data = lerCluster(index);					 //ler o diretório raiz
	for (int i = 1; i <= strlen(diretorio); i++)
	{																																					 //percorre a string diretorio
		if (diretorio[i] == '/' || diretorio[i] == '\0' || diretorio[i] == '\n') //se encontrar algum desses caracters, um nome de diretório foi encontrado
		{																																				 //fim do nome de um diretorio
			dirAtual[j] = '\0';																										 //dirAtual que recebe o nome do diretorio recebe o \0
			for (k = 0; k < 32; k++)
			{ //percoore as 32 entradas de diretório do cluster atual
				if (strcmp(data.dir[k].filename, dirAtual) == 0 && data.dir[k].first_block != 0)
				{ // verifica se acha um diretorio no cluster com o nome do dirAtual
					if (data.dir[k].attributes == 1)
					{																	 //existe diretorio
						index = data.dir[k].first_block; //recebe o index do cluster encontrado
						data = lerCluster(index);				 //ler o cluster encontrado
						break;
					}
					else
					{ //arquivo
						if (numDiretorios == 0 && procura == 3)
						{ //se a pesquisa estiver no fim e for para achar um arquivo retorna o cluster
							return index;
						}
					}
				}
			}
			if (procura == 1)
			{ //se for uma procura por um diretorio que será criado não deverá encontrar, ou se necessitar do diretório pai de algum diretório
				if (k == 32 || numDiretorios == 0)
				{ //k == 32 significa que não achou o atualDir, e numDiretorios == 0 significa que chegou ao fim do caminho
					if (numDiretorios != 0)
					{ //se o k == 32, mas não estiver no fim do arquivo, logo o diretorio não foi encontrado
						printf("DIRETORIO NAO ENCONTRADO\n");
						return -1;
					}
					if (k < 32)
					{ //encontrou o diretorio, retorna o pai
						return indexPai;
					}
					return index;
				}
			}
			else if (procura == 2)
			{ // se for uma procura por um diretorio que ja existe deverá encontrar
				if (k == 32 && numDiretorios == 0)
				{ // se estiver no fim do caminho e k == 32, não encontrou o cluster de algum no caminho
					printf("DIRETORIO NAO ENCONTRADO\n");
					return -1;
				}
				else if (k < 32 && numDiretorios == 0)
				{ // percorreu todo cluster e encontrou um diretorio com nome de dirAtual
					return index;
				}
			}
			numDiretorios--;	//encontrou mais um diretorio, logo o num de diretorios que devem ser porcurados diminui 1
			j = 0;						//variavel auxiliar para preencher dirAtual
			indexPai = index; //salva o index atual, que será o index pai do próximo
		}
		else
		{
			dirAtual[j] = diretorio[i]; //preecnhe dirAtual com o nome do diretório
			j++;
		}
	}
	return -1;
}

int getNumDiretorios(char *caminho)
{												 //função usada para contar o número de diretórios
	int numDiretorios = 0; // variável para contar o número de diretórios inicializada com 0
	int i = 0;						 //variável para percorrer a string
	do
	{ //compara a string recebida na posição i, até que a string chegue ao fim. Sempre pelo menos a primeira posição é comparada
		if (caminho[i] == '/')
		{ //caso encontre um caractere '/', aumenta um no contador de diretórios
			numDiretorios++;
		}
		i++;
	} while (caminho[i] != '\0' && caminho[i] != '\n');
	return numDiretorios - 1; // se retira um no contador porque o primeiro caractere '/' se refere ao diretório raiz
}

//clusters
data_cluster lerCluster(int index)
{
	FILE *arq = fopen("fat.part", "rb"); //abre o arquivo o arquivo fat.part
	data_cluster cluster;
	if (arq == NULL)
	{
		printf("ERRO ao abrir arquivo fat\n");
		exit(1);
	}
	memset(&cluster, 0x00, CLUSTER_SIZE);				//cria um cluster
	fseek(arq, index * CLUSTER_SIZE, SEEK_SET); //move o ponteiro para a posição index
	fread(&cluster, CLUSTER_SIZE, 1, arq);			//le o cluster
	fclose(arq);
	return cluster;
}

void salvarCluster(int index, data_cluster cluster)
{
	FILE *arq = fopen("fat.part", "rb+"); // abre o arquivo para atualização
	if (arq == NULL)
	{
		printf("ERRO ao abrir arquivo fat\n");
		exit(1);
	}
	fseek(arq, index * CLUSTER_SIZE, SEEK_SET); // Aponta para o cluster do index
	fwrite(&cluster, CLUSTER_SIZE, 1, arq);			//salva cluster na memória
	fclose(arq);
}

//strings
void separaString(char *string1, char *string2, char *string3, char *separador)
{ //recebe uma string e separa a mesma em duas em relação à um caractere separador
	if (strcmp(string1, "") == 0)
	{
		strcpy(string2, "");
		strcpy(string3, "");
		return;
	}
	int tam = strlen(string1);
	char *aux;
	strcpy(string2, strtok(string1, separador)); //string2 recebe um string que vai do inicio da string1 até a posição da primeira ocorrencia do caractere separador
	if (tam == strlen(string2))
	{ //se a nova string e a antiga tiverem o mesmo tamanho, significa que não é necessário separar a string recebida em duas. Por isso a string3 fica vazia
		strcpy(string3, "");
	}
	else
	{
		if (strcmp(separador, "/") == 0)
		{ //caso o caractere separador for "/", que é passado nas chamadas da função separaString nas funções write e append, é necessário que a string3 possua o caractere no seu inicio. Strtok não coloca o caractere separador em nenhuma string da separação, por isso o caractere é colocado antes de se dividir
			strcpy(string3, "/");
			aux = strtok(NULL, "\n"); //string aux recebe a string retornada na chamada da função strtok, partindo do ponto onde a ultima chamada da função parou até que se chegue ao final da string. Foi necessário um auxiliar porque só é possível usar strtok, partindo do ponto onde parou a ultima chamada  do strtok, uma vez
			if (aux == NULL)
			{
				strcpy(string3, ""); //caso aux seja nulo a string 3 recebe um string vazia
			}
			else
			{
				strcat(string3, aux); //adiciona aux no final da string3.
			}
		}
		else
		{														//caso o separador não seja '/'
			aux = strtok(NULL, "\n"); //string aux recebe a string retornada na chamada da função strtok, partindo do ponto onde a ultima chamada da função parou até que se chegue ao final da string. Foi necessário um auxiliar porque só é possível usar strtok, partindo do ponto onde parou a ultima chamada  do strtok, uma vez
			if (aux == NULL)
			{
				strcpy(string3, ""); //caso aux seja nulo a string 3 recebe um string vazia
			}
			else
			{
				strcpy(string3, aux); //caso aux não seja nulo string3 recebe aux
			}
		}
	}
}

data_cluster *quebrarStringClusters(char *string, int *numClusters)
{ //a função quebra a string recebida em quantos clusters forem necessários
	data_cluster *clusters;
	int tamString, numClustersInteiros, numClustersFalta;
	tamString = strlen(string);
	numClustersInteiros = tamString / CLUSTER_SIZE; // a variavel recebe o numero de clusters que serão ocupados por inteiro pela string
	numClustersFalta = tamString % CLUSTER_SIZE;		// a variavel recebe o numero do quanto será necessário ocupar de um cluster para receber o restante da string

	if (numClustersInteiros != 0)
	{ //caso a string ocupe um ou mais clusters inteiros
		int restante = 0, i = 0;
		if (numClustersFalta >= 0)
		{
			restante = 1; //caso a string precise de mais um cluster para o restante da string, reestante recebe 1
		}
		clusters = (data_cluster *)malloc((numClustersInteiros + restante) * CLUSTER_SIZE); //aloca espaço para o vetor de clusters
		for (i = 0; i < numClustersInteiros; i++)
		{																																				//separa a string nos clusters
			memset(&(clusters)[i], 0x00, CLUSTER_SIZE);														//preenche o cluster com 0x00
			memcpy(&(clusters)[i].data, &string[(CLUSTER_SIZE)*i], CLUSTER_SIZE); //copia a string para dentro do cluster
			((clusters)[i].data)[CLUSTER_SIZE] = '\0';														//finzaliza a string neste cluster
		}
		if (restante == 1)
		{																																							//caso seja mais um cluster para colocar o restante da string, isso é feito
			memset(&(clusters)[i], 0x00, CLUSTER_SIZE);																	//preenche o cluster com 0x00
			memcpy(&(clusters)[i].data, &string[i * (CLUSTER_SIZE)], numClustersFalta); //copia a string para dentro do cluster
			(clusters)[i].data[numClustersFalta] = '\0';																//finzaliza a string neste cluster
		}
		(*numClusters) = numClustersInteiros + restante; //número de clusters total recebe o número de clusters utilizados
		return clusters;
	}
	else
	{ //ocupa apenas um cluster
		clusters = (data_cluster *)malloc(CLUSTER_SIZE);
		memset(clusters, 0x00, CLUSTER_SIZE);			 //preenche o cluster com 0x00
		memcpy(&(clusters)[0], string, tamString); //copia a string para dentro do cluster
		(*numClusters) = 1;
		return clusters;
	}
}

// fat
void atualizarFat()
{ //atualiza no arquivo apenas a tabela fat
	FILE *arq = fopen("fat.part", "rb+");
	if (arq == NULL)
	{
		printf("ERRO ao abrir arquivo fat\n");
		exit(1);
	}
	fseek(arq, CLUSTER_SIZE, SEEK_SET);				//Aponta para o FAT após o boot_block de 1024 bytes
	fwrite(fat, sizeof(uint16_t), 4096, arq); //Salva fat
	fclose(arq);
}