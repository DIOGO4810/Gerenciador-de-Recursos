#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"

#define MaxTokensSize 12

struct parser
{
    FILE *file;

    char *line;

    char **tokens;

    int offset;
};

struct cpuStage{
    int user, nice, system, idle, iowait, irq, softirq, steal;
};

// Função que cria e aloca memoria para a estrutura e os campos do Parser
Parser *newParser(char *diretoria)
{
    Parser *parserE = malloc(sizeof(struct parser));
    parserE->file = fopen(diretoria, "r");
    parserE->tokens = malloc(MaxTokensSize * sizeof(char *));
    parserE->line = NULL;
    parserE->offset = 0;

    return parserE;
}

Parser *parser(Parser *parserE, char *diretoria, int flag)
{
    if (flag)
    {
        parserE->file = fopen(diretoria, "r");
        if (fseek(parserE->file, parserE->offset, SEEK_CUR) != 0)
            printf("erro");
    }
    size_t len = 0;
    char *line = NULL;

    if (getline(&line, &len, parserE->file) == -1)
    {
        // Dá free de linha e return caso n haja mais linhas no ficheiro
        free(line);
        parserE->tokens[0] = NULL;
        return parserE;
    }

    // Remove a nova linha no final, se existir
    if (line[strlen(line) - 1] == '\n')
    {
        line[strlen(line) - 1] = '\0';
    }

    parserE->line = strdup(line);

    if (flag)
        parserE->offset += strlen(parserE->line) + 1;

    char *lineCopy = line;

    int i = 0;

    // Divide a linha em tokens usando strsep
    char *token = strsep(&lineCopy, " ");
    while (token != NULL && i < MaxTokensSize)
    {
        // Armazenar o token no array
        parserE->tokens[i++] = token;
        token = strsep(&lineCopy, " ");
    }

    fclose(parserE->file);

    return parserE;
}

// Função que dá free da memoria para a estrutura e os campos do Parser
void freeParser(Parser *parserE)
{
    free(parserE->tokens);
    free(parserE);
}

char *pegaLinha(Parser *parserE)
{

    size_t len = 0;
    char *line = NULL;
    if (getline(&line, &len, parserE->file) == -1)
    {
        // Dá free de linha e return de NULL caso n haja mais linhas no ficheiro
        free(line);
        return NULL;
    }

    // Remove a nova linha no final, se existir
    if (line[strlen(line) - 1] == '\n')
    {
        line[strlen(line) - 1] = '\0';
    }
    fclose(parserE->file);
    parserE->offset += strlen(line);
    parserE->offset += 1;
    return line;
}

char **getTokens(Parser *parserE)
{
    if (parserE->tokens[0] == NULL)
        return NULL;

    return parserE->tokens;
}

void coresInfo()
{

    Parser *parserStat = newParser("/proc/stat");
    char *totalCoreLine = pegaLinha(parserStat);

    int user, nice, system, idle, iowait, irq, softirq, steal;
    int prevUser, prevNice, prevSystem, prevIdle, prevIowait, prevIrq, prevSoftirq, prevSteal;
    int i = 0;
    while (1)
    {
        i++;

        parserStat = parser(parserStat, "/proc/stat", 1);

        if (parserStat->tokens == NULL || strcmp(parserStat->tokens[0], "intr") == 0)
        {
            freeParser(parserStat);
            break;
        }

        prevUser = atoi(parserStat->tokens[1]);
        prevNice = atoi(parserStat->tokens[2]);
        prevSystem = atoi(parserStat->tokens[3]);
        prevIdle = atoi(parserStat->tokens[4]);
        prevIowait = atoi(parserStat->tokens[5]);
        prevIrq = atoi(parserStat->tokens[6]);
        prevSoftirq = atoi(parserStat->tokens[7]);
        prevSteal = atoi(parserStat->tokens[8]);

        sleep(1);
        parserStat->file = fopen("/proc/stat", "r");
        fseek(parserStat->file, parserStat->offset - strlen(parserStat->line) - 1, SEEK_SET);

        parserStat = parser(parserStat, "/proc/stat", 0);

        user = atoi(parserStat->tokens[1]);
        nice = atoi(parserStat->tokens[2]);
        system = atoi(parserStat->tokens[3]);
        idle = atoi(parserStat->tokens[4]);
        iowait = atoi(parserStat->tokens[5]);
        irq = atoi(parserStat->tokens[6]);
        softirq = atoi(parserStat->tokens[7]);
        steal = atoi(parserStat->tokens[8]);

        unsigned long deltaActive = (user + nice + system + irq + softirq + steal) -
                                    (prevUser + prevNice + prevSystem + prevIrq + prevSoftirq + prevSteal);

        unsigned long deltaTotal = (user + nice + system + idle + iowait + irq + softirq + steal) -
                                   (prevUser + prevNice + prevSystem + prevIdle + prevIowait + prevIrq + prevSoftirq + prevSteal);


        double cpuUsage = (deltaTotal > 0) ? (100.0 * deltaActive / deltaTotal) : 0.0;

        printf("Thread%d usage:%.2f%%\n",i,cpuUsage);

        
                           
    }
}