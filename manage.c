#include <sys/sysinfo.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <sys/statvfs.h>
#include <stdlib.h>



#define ToGB 1024/1024/1024
#define ToBytes 1024 * 1024
#define ToCº 1000
#define ToPerc 100
#define size 128
#define ToMiliampere 1000

typedef struct sysinfo systems;

typedef struct statvfs disk;




void manageCPU(systems *info,char stringTemp[],FILE *files){
        //Número de nucleos do processador
        int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

        // Calcular a carga média em porcentagem
        float load_avg_1 = (info->loads[0] / pow(2,16)) / num_cpus *ToPerc;
        float load_avg_5 = (info->loads[1] / pow(2,16)) / num_cpus *ToPerc;
        float load_avg_15 = (info->loads[2] / pow(2,16)) / num_cpus *ToPerc;

        // Pesos para média ponderada
        float weight_1 = 0.6;  // Peso para carga média de 1 minuto
        float weight_5 = 0.3;  // Peso para carga média de 5 minutos
        float weight_15 = 0.1; // Peso para carga média de 15 minutos

        // Calcular média ponderada
        float weighted_avg = (load_avg_1 * weight_1) + (load_avg_5 * weight_5) + (load_avg_15 * weight_15);


        printf("Uso médio do processador: %f%%\n", weighted_avg);

    
    files = fopen("/sys/class/thermal/thermal_zone7/temp","r");
    

   if (fgets(stringTemp, size, files) != NULL) {
        int temp = atoi(stringTemp) / ToCº;
        printf("Temperatura atual do processador: %d°C\n", temp);
    } else {
        printf("Não foi possível ler a temperatura do processador.\n");
    }
    
    // Fecha o arquivo
    fclose(files);

    

}






void manageRAM() {
    FILE *file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        perror("Não foi possível abrir /proc/meminfo");
        return;
    }

    unsigned long totalRAM = 0;
    unsigned long availableRAM = 0;

    // Lê e processa cada linha do arquivo /proc/meminfo
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "MemTotal: %lu kB", &totalRAM) == 1) {
            // Total RAM em kB
        } else if (sscanf(line, "MemAvailable: %lu kB", &availableRAM) == 1) {
            // Memória disponível em kB
        }
    }

    fclose(file);

    // Converte para GB e exibe
    printf("Quantidade total de RAM: %.2f GB\n", (double)totalRAM / 1024 / 1024);
    printf("Quantidade de RAM disponível: %.2f GB\n", (double)availableRAM / 1024 / 1024);
}







void manageUpTime (systems *info){
    unsigned long UpHours = (info->uptime)/60/60;
    unsigned long UpMin = (info->uptime)/60;
    unsigned long remainingMinutes = UpMin % 60;
    FILE *batteryNow = fopen("/sys/class/power_supply/BAT1/charge_now", "r");
    FILE *batteryFull = fopen("/sys/class/power_supply/BAT1/charge_full", "r");

    char fullBattery[256];
    float intfullBattery = atoi(fgets(fullBattery,sizeof(fullBattery),batteryFull));
    char currentBattery[256];
    float intcurrentBattery = atoi(fgets(currentBattery,sizeof(currentBattery),batteryNow));
    // printf("%f\n",intcurrentBattery/intfullBattery);
    float waistedBattery = 100- (intcurrentBattery/intfullBattery)*100;
    if(UpHours < 1)
    {
        printf("O sistema está aberto a %ld minutos\n",UpMin);   
        printf("O sistema gastou %.2f%% de bateria nesse tempo\n",waistedBattery);
    }
    else
    {
        printf("O sistema está aberto a %ld horas e %ld minutos \n",UpHours,remainingMinutes);
        printf("O sistema gastou %.2f%% de bateria nesse tempo\n",waistedBattery);

    }

    fclose(batteryNow);
    fclose(batteryFull);
}


void manageDisk (disk *infoDisk){
    //Tamanho de cada bloco, numero de blocos, e numero de blocos livre.
    double blockSize = infoDisk->f_bsize;
    double nBlocos = infoDisk->f_blocks;
    double freeBlocos = infoDisk->f_bfree;

    
    double totalSpace= nBlocos * blockSize / ToGB;
    double freeSpace = blockSize * freeBlocos /ToGB;
    double spaceUsed = totalSpace - freeSpace;

    printf("Quantidade de espaço do sistema: %.2fGB \n",totalSpace);
    printf("Quantidade de espaço livre no sistema: %.2fGB \n",freeSpace);
    printf("Quantidade de espaço usado no sistema: %.2fGB \n",spaceUsed);

}







void manageMemLeak (systems *info){
    printf("A analisar o possivel vazamento de memória ...\n");

    long int antes = info->freeram;
    sleep(10);
    sysinfo(info);
    long int depois = info->freeram;
     
 if ((antes - depois) > 80 * ToBytes) {
        printf("Alerta: Possível vazamento de memória detectado.\n");
    } else {
        printf("Não foi detectado vazamento de memória.\n");
    }



}

void manageTemp(){
    int tempCPU;
    int tempSSD;
    char stringTempCPU[size];
    char stringTempSSD[size];
    FILE *filesCPU = NULL;
    FILE *filesSSD = NULL;

    printf("A seguir estão as temperaturas atuais do sistema, atualizadas a cada segundo durante um período de 8 segundos:\n");
    sleep(3);
    printf("O processador está a:\t\t O SSD está a:\n");

    for (int i = 0; i < 8; i++)
    { 
    
    filesCPU = fopen("/sys/class/thermal/thermal_zone7/temp","r");
    filesSSD = fopen("/sys/class/hwmon/hwmon4/temp1_input","r");


    if (fgets(stringTempCPU,size,filesCPU) != NULL && fgets(stringTempSSD,size,filesSSD) != NULL )
    {
        tempCPU = atoi(stringTempCPU)/ToCº;
        tempSSD = atoi(stringTempSSD)/ToCº;
    }
    printf("%dº//////////////////// %d graus  %dº//////////// %d graus\n",i+1,tempCPU,i+1,tempSSD);
    sleep(1);
    
    }
    // Fecha os arquivos
    fclose(filesCPU);
    fclose(filesSSD);

}

void manageBattery () {

    FILE *batteryDischarge = fopen("/sys/class/power_supply/BAT1/current_now", "r");
    

    float discharge = 0;
    char dischargeString[256];
    if(fgets(dischargeString,sizeof(dischargeString),batteryDischarge) != NULL){
        discharge = strtod(dischargeString,NULL)/ToMiliampere;
    }
    fclose(batteryDischarge);

    printf("O PC está a descarregar %.1f mA\n",discharge);




}



void manage (int mode ){
    systems info;
    sysinfo(&info);
    disk infoDisk;
    statvfs("/",&infoDisk);
    char stringTemp[128];
    FILE *files = NULL;


    switch (mode)
    {
    case 1 :manageBattery() ;break;

    case 2: manageRAM();break;

    case 3:manageUpTime(&info);break;

    case 4:manageDisk(&infoDisk);break;

    case 5:manageCPU(&info,stringTemp,files);break;

    case 6:manageMemLeak(&info) ;break;

    case 7:manageTemp() ;break;

    default:
        printf("Invalid mode\n");
        break;

    }


}

void infoPrints (){
    printf("Selecione o tipo de informação que deseja consultar:\n");
    printf("1 - Informações da bateria\n");
    printf("2 - RAM\n");
    printf("3 - Tempo de atividade\n");
    printf("4 - Espaço em disco\n");
    printf("5 - Uso do CPU\n");
    printf("6 - Verificação de vazamento de memória\n");
    printf("7 - Temperaturas do sistema\n");
}

int validaInput (char* input){
    int length = strlen(input);
    if(length != 2)return 0;
    if(!isdigit(input[0])) return 0;
    return 1;

}


int main() {
    char input[256];
    char mode;
    
    infoPrints();

    while (1) {
        printf("\n\nInsira ENTER caso queira sair Insira '?' caso queira ver os modos novamente\n");
        printf("Escreva o número correspondente à opção desejada: ");

        if(fgets(input,sizeof(input),stdin) == NULL) {
            printf("Modo Inválido\n");
            continue;
        }
        mode = input[0];

        // If ENTER key is pressed (ASCII 10), exit the loop
        if (mode == 10) {
            break;
        }

        // If ? key is pressed (ASCII 63), print Info again
        if(mode == 63){
            infoPrints();
            continue;
        }


        int isValid = validaInput(input);
        if(!isValid){
            printf("Modo Inválido\n");
            continue;
        }


        // Convert character to corresponding integer value
        mode = mode - '0';

        manage(mode);
        
    }
    
    
    return 0;
}


