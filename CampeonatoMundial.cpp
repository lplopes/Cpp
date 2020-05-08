/*
Tratase de un supuesto campeonato mundial con N clubs, en el cual el campe�n ser� el que obtener m�s puntos en juegos en casa y como visitante.
El programa obtiene el archivo Jogos.txt, conteeniendo los juegos con menor distancia de viajes para los equipos, de tal manera que cada una salga
solamente una vez para viajar al rededor del mundo para jugar todos sus partidos como visitante. Este programa utiliza c�lculo de permutaciones y
procesamiento paralelo.
*/

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <thread>
#include <time.h>
#include <stdio.h>

#define raioTerra 6.371e+6
#define pi 3.1415926535897932384626433832795

using namespace std;

class cf
{
    public:
        int adv; //Advers�rio
        bool casa; //Se o jogo � em casa
        double d; //Dist�ncia viajada para este jogo
};

class equipe
{
    public:
        char* federacao;
        char* nome;
        double pos[2];
};

class config
{
    public:
        int n, np, * it, jogos, ** pp;
        unsigned long long p;
        double dmin;
        cf** t;
        equipe* time;
};

bool verif(config inicial, cf** t, unsigned long long* ip, int i)
{
    //Declara��o das vari�veis
    int j;
    bool flag = true;

    //Montar linha da tabela
    for(j = 0; j < inicial.jogos; j++)
        t[i][j] = inicial.t[i][inicial.pp[ip[i]][j]];

    //Verificar linhas anteriores
    for(j = 0; j < inicial.jogos; j++)
        if(t[i][j].adv < i && (t[t[i][j].adv][j].adv != i || t[i][j].casa == t[t[i][j].adv][j].casa))
        {
            flag = false;
            break;
        }

    //Incrementar os �ndices da permuta��o
    if(!flag)
    {
        ip[i]++;
        for(j = i + 1; j < inicial.n; j++)
            ip[j] = 0;
    }
    //Verificar pr�xima linha
    else if(i < inicial.n - 1)
        flag = verif(inicial, t, ip, i + 1);
    //Incrementar �ndice da �ltima linha
    else
        ip[inicial.n - 1]++;

    //Corrigir os �ndices das permuta��es
    for(j = inicial.n - 1; j > 0; j--)
        if(ip[j] >= inicial.p)
        {
            ip[j] = 0;
            ip[j - 1]++;
        }

	//Retornar verifica��o
    return flag;
}

double dist(double *pos1, double *pos2)
{
    //Declara��o das vari�veis
    int i;
    double p1[3], p2[3], qh, ang, d;

    //Determinar os vetores das posi��es
    p1[0] = raioTerra * cos(pos1[0] / 180 * pi) * cos(pos1[1] / 180 * pi);
    p1[1] = raioTerra * cos(pos1[0] / 180 * pi) * sin(pos1[1] / 180 * pi);
    p1[2] = raioTerra * sin(pos1[0] / 180 * pi);
    p2[0] = raioTerra * cos(pos2[0] / 180 * pi) * cos(pos2[1] / 180 * pi);
    p2[1] = raioTerra * cos(pos2[0] / 180 * pi) * sin(pos2[1] / 180 * pi);
    p2[2] = raioTerra * sin(pos2[0] / 180 * pi);

    //Calcular o vetor resultante
    for(i = 0; i < 3; i++)
        p1[i] -= p2[i];

    //M�dulo do vetor resultante
    qh = 0;
    for(i = 0; i < 3; i++)
        qh += pow(p1[i], 2);

    //Determinar o �ngulo entre as duas cidades
    ang = acos(1 - qh / (2 * pow(raioTerra, 2)));

    //Calcular a dist�ncia
    d = ang * raioTerra / 1000;

    return d;
}

void* parc(config inicial, int parte)
{
	//Declara��o das vari�veis
	int i, j;
	unsigned long long* ip;
	bool flag;
	double *d;
	cf** t;
	ofstream output;
	string arq = "Jogos_XX.txt";

	//Aloca��o de mem�ria
	t = (cf**)calloc(inicial.n, sizeof(cf*));
	for (i = 0; i < inicial.n; i++)
		t[i] = (cf*)calloc(inicial.jogos, sizeof(cf));
	ip = (unsigned long long*) calloc(inicial.n, sizeof(unsigned long long));
	d = (double*) calloc(inicial.n + 1, sizeof(double));

	//Calcular a permuta��o inicial
	ip[0] = (parte - 1) * inicial.p / inicial.np;

	//Montar, verificar e imprimir todas as tabelas do intervalo
	arq.replace(6, 2, to_string(parte));
	while (ip[0] < parte * inicial.p / inicial.np)
	{
		//Criar tabela para a permita��o ip
        for(i = 0; i < inicial.jogos; i++)
            t[0][i] = inicial.t[0][inicial.pp[ip[0]][i]];

		//Verificar coincid�ncias e incrementar os �ndices da permuta��o
		flag = verif(inicial, t, ip, 1);

		//Calcular dist�ncias das viagens
		if(flag)
		{
		    for(i = 0; i < inicial.n; i++)
            {
                for(j = 0; j < inicial.jogos; j++)
                {
                    //Jogos fora
                    if(!t[i][j].casa)
                        if(j == 0 || (j > 0 && t[i][j - 1].casa)) //Se � o primeiro jogo fora
                            t[i][j].d = dist(inicial.time[i].pos, inicial.time[t[i][j].adv].pos);
                        else
                            t[i][j].d = dist(inicial.time[t[i][j - 1].adv].pos, inicial.time[t[i][j].adv].pos); //Se � continua��o da jornada
                    else
                        t[i][j].d = 0;

                    //Viagem de retorno
                    if(j > 0 && t[i][j].casa && !t[i][j - 1].casa)
                        t[i][j].d = dist(inicial.time[i].pos, inicial.time[t[i][j - 1].adv].pos);
                }

                //Se o �ltimo jogo for fora
                if(!t[i][inicial.jogos - 1].casa)
                    t[i][0].d = dist(inicial.time[i].pos, inicial.time[t[i][inicial.jogos - 1].adv].pos);
            }

            //Dist�ncia percorrida por cada time
            for(i = 0; i < inicial.n; i++)
            {
                d[i] = 0;
                for(j = 0; j < inicial.jogos; j++)
                    d[i] += t[i][j].d;
            }

            //Dist�ncia total
            d[inicial.n] = 0;
            for(i = 0; i < inicial.n; i++)
                d[inicial.n] += d[i];

            //Comparar com as dist�ncias anteriores
            if(!inicial.dmin)
                inicial.dmin = d[inicial.n];
            else if(d[inicial.n] < inicial.dmin)
            {
                inicial.dmin = d[inicial.n];
                inicial.it[parte - 1] = 0;
                char* rarq = new char[arq.length()];
                memcpy(rarq, arq.c_str(), arq.length());
                remove(rarq);
                delete[] rarq;
            }
            else if(d[inicial.n] > inicial.dmin)
                flag = false;
		}

		//Escrever tabela
		if(flag)
        {
            output.open(arq, ios_base::app);
			output << "Tabela " << parte << "." << inicial.it[parte - 1] + 1 << ":\n";
			for (i = 0; i < inicial.n; i++)
			{
				output << inicial.time[i].nome << ": ";
				for(j = 0; j < inicial.jogos; j++)
				{
					if(t[i][j].casa)
						if(t[i][j].adv == i)
							output << "sem jogo";
						else
							output << inicial.time[t[i][j].adv].nome << "(c)";
					else
						output << inicial.time[t[i][j].adv].nome << "(f)";
					if(j < inicial.jogos - 1)
						output << ", ";
				}
                output << " (";
                output << fixed << setprecision(2) << d[i] << " km)" << endl;
			}
			output << "\n";
			output.close();
			inicial.it[parte - 1]++;
        }
	}

	//Liberar mem�ria
	free(t);
	free(ip);
	free(d);

	return NULL;
}

int main()
{
	//Declara��o das vari�veis
	int i, j, k, nopl;
	unsigned long long l, p;
	double cput;
	bool flag;
	string arq;
	char c;
	config inicial;
	clock_t start, end;
	ofstream output1;
	ifstream output2;
	thread* parte;

	//Entrada de dados
	cout << "Numero de equipes: ";
	cin >> inicial.n;
	cout << "Janelas sem jogo: ";
	cin >> nopl;

	//Tempo de processamento
	start = clock();

	//N�mero de jogos
	inicial.jogos = 2 * (inicial.n - 1) + nopl;

	//Total de permuta��es
	inicial.p = 1;
	for (i = 2; i <= inicial.jogos; i++)
		inicial.p *= i;

	//N�mero de processadores
	inicial.np = thread::hardware_concurrency();

	//Aloca��o de mem�ria
	inicial.t = (cf**)calloc(inicial.n, sizeof(cf*));
	for (i = 0; i < inicial.n; i++)
		inicial.t[i] = (cf*)calloc(inicial.jogos, sizeof(cf));
	inicial.pp = (int**)calloc(inicial.p, sizeof(int*));
	for (i = 0; i < inicial.p; i++)
		inicial.pp[i] = (int*)calloc(inicial.jogos, sizeof(int));
	parte = (thread*)calloc(inicial.np, sizeof(thread));
	inicial.time = (equipe*)calloc(inicial.n, sizeof(equipe));
	for(i = 0; i < inicial.n; i++)
	{
        inicial.time[i].federacao = (char *)calloc(9, sizeof(char));
	    inicial.time[i].nome = (char *)calloc(21, sizeof(char));
	}

	//Entrada de dados do arquivo
	output2.open("Equipes.txt");
	for(i = 0; i < inicial.n; i++)
    {
        //Federa��o
        j = 0;
        while(true)
        {
            output2.get(inicial.time[i].federacao[j]);
            if(inicial.time[i].federacao[j] == ' ')
            {
                inicial.time[i].federacao[j] = '\0';
                break;
            }
            else
                j++;
        }
        for(j = 0; j < sizeof(inicial.time[i].federacao); j++)
            if(inicial.time[i].federacao[j] == '_')
                inicial.time[i].federacao[j] = ' ';

        //Equipe
        j = 0;
        while(true)
        {
            output2.get(inicial.time[i].nome[j]);
            if(inicial.time[i].nome[j] == ' ')
            {
                inicial.time[i].nome[j] = '\0';
                break;
            }
            else
                j++;
        }
        for(j = 0; j < sizeof(inicial.time[i].nome); j++)
            if(inicial.time[i].nome[j] == '_')
                inicial.time[i].nome[j] = ' ';

        //Posi��o geogr�fica
        output2 >> inicial.time[i].pos[0];
        output2 >> inicial.time[i].pos[1];

        //Retorno de linha
        while(true)
            if(output2.get() == '\n')
                break;
    }
	output2.close();

	//Imprimir na tela a lista dos campe�es
	cout << "\nCampeoes pelas federacoes:" << endl;
	for(i = 0; i < inicial.n; i++)
        cout << inicial.time[i].federacao << ": " << inicial.time[i].nome << endl;
    cout << "\n";

	//Montar tabela inicial
	for (i = 0; i < inicial.n; i++)
	{
		for (j = 0; j < inicial.n - 1; j++)
		{
			inicial.t[i][j].adv = j;
			inicial.t[i][j].casa = false;
		}
		for (j = inicial.n - 1; j < 2 * (inicial.n - 1); j++)
		{
			inicial.t[i][j].adv = j - inicial.n + 1;
			inicial.t[i][j].casa = true;
		}
		for (j = 2 * (inicial.n - 1); j < inicial.jogos; j++)
		{
			inicial.t[i][j].adv = i;
			inicial.t[i][j].casa = true;
		}
	}

	//Corrigir �ndice
	for (i = 0; i < inicial.n; i++)
		for (j = 0; j < 2 * (inicial.n - 1); j++)
			if (inicial.t[i][j].adv >= i)
				inicial.t[i][j].adv++;

	//Lista de permuta��es poss�veis
	for (i = 0; i < inicial.jogos; i++)
		inicial.pp[0][i] = i;
	for (i = 1; i < inicial.p; i++)
	{
		for (j = 0; j < inicial.jogos; j++)
			inicial.pp[i][j] = inicial.pp[i - 1][j];
		do
		{
			inicial.pp[i][inicial.jogos - 1]++;
			for (j = inicial.jogos - 1; j > 0; j--)
				if (inicial.pp[i][j] >= inicial.jogos)
				{
					inicial.pp[i][j] = 0;
					inicial.pp[i][j - 1]++;
				}
			flag = false;
			for (j = 0; j < inicial.jogos - 1; j++)
			{
				for (k = j + 1; k < inicial.jogos; k++)
					if (inicial.pp[i][j] == inicial.pp[i][k])
					{
						flag = true;
						break;
					}
				if (flag)
					break;
			}
		} while (flag);
	}

	//Eliminar permuta��es desnecess�rias
	p = 0;
	for (l = 0; l < inicial.p; l++)
	{
		//Verificar sequ�ncia de jogos fora
		j = 0;
		while (inicial.pp[l][j] >= inicial.n - 1)
			j++;
		flag = true;
		for (k = j + 1; k < j + inicial.n - 1; k++)
			if (inicial.pp[l][k] >= inicial.n - 1)
			{
				flag = false;
				break;
			}

		//Verificar rodadas sem jogos
		if (flag)
		{
			k = 2 * (inicial.n - 1);
			for (j = 0; j < inicial.jogos; j++)
				if (inicial.pp[l][j] == k)
					k++;
				else if (inicial.pp[l][j] > k)
				{
					flag = false;
					break;
				}
		}

		//Reescrever vetor de permuta��es
		if (flag)
		{
			for (j = 0; j < inicial.jogos; j++)
				inicial.pp[p][j] = inicial.pp[l][j];
			p++;
		}
	}

	//Redimensionar vari�veis
	inicial.p = p;
	if (inicial.np > inicial.p)
		inicial.np = inicial.p;
	inicial.pp = (int**)realloc(inicial.pp, inicial.p * sizeof(int*));
	for (i = 0; i < inicial.p; i++)
		inicial.pp[i] = (int*)realloc(inicial.pp[i], inicial.jogos * sizeof(int));
	cout << "Permutacoes calculadas para " << inicial.jogos << " rodadas.\n";

	//Valor inicial da quantidade de tabelas encontradas
	inicial.it = (int*)calloc(inicial.np, sizeof(int));
	for (i = 0; i < inicial.np; i++)
		inicial.it[i] = 0;
    inicial.dmin = 0;

	//Chamada para processamento das partes
	for (i = 0; i < inicial.np; i++)
		parte[i] = thread(parc, inicial, i + 1);

	//Aguardar todas as threads finalizarem
	for (i = 0; i < inicial.np; i++)
		parte[i].join();

	//N�mero de possibilidades
	k = 0;
	for (i = 0; i < inicial.np; i++)
		k += inicial.it[i];

	//Liberar mem�ria
	free(inicial.t);
	free(inicial.it);
	free(inicial.pp);
	free(inicial.time);

	//Escrever arquivo final
	output1.open("Jogos.txt");
	for (i = 0; i < inicial.np; i++)
	{
		arq = "Jogos_XX.txt";
		arq.replace(6, 2, to_string(i + 1));
		output2.open(arq);
		if (!output2)
			continue;
		while (c = output2.get())
			if (c != EOF)
				output1 << c;
			else
				break;
		output2.close();
		char* rarq = new char[arq.length()];
		memcpy(rarq, arq.c_str(), arq.length());
		remove(rarq);
		delete[] rarq;
	}

	//Calcula o tempo de processamento e imprime na tela
	end = clock();
	cput = ((float)(end - start)) / CLOCKS_PER_SEC;
	output1 << "Tempo de execucao: " << fixed << setprecision(4) << cput << " s.\n";
	output1 << k << " possibilidades.";
	output1.close();

	return 0;
}
