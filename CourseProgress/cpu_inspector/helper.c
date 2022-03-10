#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
typedef struct{
   char name[20];
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long guest;
    unsigned long long allbasytime;
    unsigned long long allruntime;

}CPUStat;

int countCPU(){
    int ret = 0;
    FILE * fp = fopen("/proc/stat", "r");
    char line[256] = {0};
    fgets(line, 255, fp); // remove the first line
    while(fgets(line, 255, fp) != NULL){
        line[3] = '\0';
        if(strcmp(line, "cpu") != 0)
            break;
            ret += 1;
    }
    fclose(fp);
    return ret; // remove the first line
}

CPUStat * initCPUs(int cpu_num){
    CPUStat * ret = (CPUStat *)malloc(sizeof(CPUStat)*(cpu_num + 5));
    memset(ret,0,sizeof(CPUStat)*cpu_num);

    FILE * fp = fopen("/proc/stat", "r");

    char line[256];
    fgets(line, 255, fp); // remove the first line

    for(int i = 0;i<cpu_num;i++){
        fgets(line, 255, fp);
        sscanf(line, "%s%llu%llu%llu%llu%llu%llu%llu%llu%llu",ret[i].name, &ret[i].user, &ret[i].nice, &ret[i].system,&ret[i].idle, &ret[i].iowait, &ret[i].irq, &ret[i].softirq, &ret[i].steal, &ret[i].guest);
        ret[i].allbasytime = ret[i].user + ret[i].nice + ret[i].system + ret[i].irq + ret[i].softirq;
        ret[i].allruntime = ret[i].allbasytime + ret[i].idle + ret[i].iowait + ret[i].steal + ret[i].guest;
    }

    fclose(fp);
    return ret;
}

void caculateUserRate(CPUStat former, CPUStat new){
    long long  userate = 1.0 * (new.allbasytime - former.allbasytime) /
                     (new.allruntime - former.allruntime) * 100.0;
    if(userate > 1){
        time_t t;
        time(&t);
        printf("%s%s userate=%lld%%\n\n", ctime(&t), new.name, userate);
    }
}

int main(){
    int cpu_num = countCPU();
    for(;;){
        CPUStat *formers = initCPUs(cpu_num);
        sleep(2);
        CPUStat *curs = initCPUs(cpu_num);
        for(int i=0;i< cpu_num;i++){
            caculateUserRate(formers[i],curs[i]);
        }
        printf("\n");
    }
}
