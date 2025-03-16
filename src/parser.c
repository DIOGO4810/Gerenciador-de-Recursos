#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"

#define MaxTokensSize 12

struct parser
{
    FILE *file;

    char* line;

    char **tokens;
};

struct cpuStage
{
    int user, nice, system, idle, iowait, irq, softirq, steal;
};

// Função que cria e aloca memoria para a estrutura e os campos do Parser
Parser *newParser(char *diretoria)
{
    Parser *parserE = malloc(sizeof(struct parser));
    parserE->file = fopen(diretoria, "r");
    parserE->tokens = malloc(MaxTokensSize * sizeof(char *));
    parserE->line = NULL;
    return parserE;
}

Parser *parser(Parser *parserE)
{

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

    parserE->line = line;

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

    return parserE;
}

// Função que dá free da memoria para a estrutura e os campos do Parser
void freeParser(Parser *parserE)
{
    fclose(parserE->file);
    free(parserE->tokens);
    free(parserE->line);
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

    return line;
}


void coresInfo()
{

    Parser *parserStat = newParser("/proc/stat");
    char *totalCoreLine = pegaLinha(parserStat);
    free(totalCoreLine);

    int i = 0;
    CpuStage prevStage[64];
    CpuStage currentStage[64];
    int dataOrEnd = 0;

    while (1)
    {

        CpuStage stage;

        parserStat = parser(parserStat);

        if (parserStat->tokens == NULL || strcmp(parserStat->tokens[0], "intr") == 0)
        {
            if (dataOrEnd == 0)
            {
                free(parserStat->line);
                fclose(parserStat->file);
                sleep(1);
                parserStat->file = fopen("/proc/stat", "r");
                dataOrEnd++;
                i = 0;
                char *totalCoreLine = pegaLinha(parserStat);
                free(totalCoreLine);
                continue;
            }

            if (dataOrEnd == 1)
            {
                freeParser(parserStat);
                break;
            }
        }

        stage.user = atoi(parserStat->tokens[1]);
        stage.nice = atoi(parserStat->tokens[2]);
        stage.system = atoi(parserStat->tokens[3]);
        stage.idle = atoi(parserStat->tokens[4]);
        stage.iowait = atoi(parserStat->tokens[5]);
        stage.irq = atoi(parserStat->tokens[6]);
        stage.softirq = atoi(parserStat->tokens[7]);
        stage.steal = atoi(parserStat->tokens[8]);

        if (dataOrEnd == 0)
        {
            prevStage[i] = stage;
        }
        else if (dataOrEnd == 1)
        {
            currentStage[i] = stage;
        }

        i++;
        free(parserStat->line);
    }
    printf("\n");

    for (int j = 0, newline = 0; j < i; j++, newline++)
    {

        unsigned long deltaActive = (currentStage[j].user + currentStage[j].nice + currentStage[j].system + currentStage[j].irq + currentStage[j].softirq + currentStage[j].steal) -
                                    (prevStage[j].user + prevStage[j].nice + prevStage[j].system + prevStage[j].irq + prevStage[j].softirq + prevStage[j].steal);

        unsigned long deltaTotal = (currentStage[j].user + currentStage[j].nice + currentStage[j].system + currentStage[j].irq + currentStage[j].softirq + currentStage[j].steal + currentStage[j].idle + currentStage[j].iowait) -
                                   (prevStage[j].user + prevStage[j].nice + prevStage[j].system + prevStage[j].irq + prevStage[j].softirq + prevStage[j].steal + prevStage[j].idle + prevStage[j].iowait);

        double cpuUsage = (deltaTotal > 0) ? (100.0 * deltaActive / deltaTotal) : 0.0;
        if (newline >= 4)
        {
            printf("\n\n");
            newline = 0;
        }
        printf("Thread %d :%.2f%%    ", j+1, cpuUsage);
    }
}