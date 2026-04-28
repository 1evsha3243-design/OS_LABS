#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "list.h"
#include "cpu.h"


struct node *task_list = NULL;

void fcfs_schedule(struct node *list);
void sjf_schedule(struct node *list);
void rr_schedule(struct node *list);
void priority_schedule(struct node *list);
void priority_rr_schedule(struct node *list);

void add_task(char *name, int priority, int burst) {
    Task *new_task = malloc(sizeof(Task));
    new_task->name = strdup(name);
    new_task->priority = priority;
    new_task->burst = burst;
    insert(&task_list, new_task);
}


void display_tasks() {
    struct node *current = task_list;
    if (current == NULL) {
        printf("\nНет задач для отображения.\n");
        return;
    }
    
    printf("\nЗАДАЧИ\n");
    while (current != NULL) {
        printf("%s, %d, %d\n", 
               current->task->name, 
               current->task->priority, 
               current->task->burst);
        current = current->next;
    }
    
}

// fcfs
void fcfs_schedule(struct node *list) {
    struct node *current = list;
    printf("\nFCFS (First Come First Served)\n");
    while (current != NULL) {
        run(current->task, current->task->burst);
        current = current->next;
    }
    printf("\n\n");
}

// sjf
void sjf_schedule(struct node *list) {
    int count = 0;
    struct node *temp = list;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    
    if (count == 0) return;
    
    Task **tasks = malloc(count * sizeof(Task*));
    temp = list;
    for (int i = 0; i < count; i++) {
        tasks[i] = temp->task;
        temp = temp->next;
    }
    
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (tasks[i]->burst > tasks[j]->burst) {
                Task *swap = tasks[i];
                tasks[i] = tasks[j];
                tasks[j] = swap;
            }
        }
    }
    
    printf("\nSJF (Shortest Job First)\n");
    for (int i = 0; i < count; i++) {
        run(tasks[i], tasks[i]->burst);
    }
    printf("\n\n");
    
    free(tasks);
}

// rr
void rr_schedule(struct node *list) {
    #define QUANTUM 10
    
    int count = 0;
    struct node *temp = list;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    
    if (count == 0) return;
    
    Task **tasks = malloc(count * sizeof(Task*));
    temp = list;
    for (int i = 0; i < count; i++) {
        tasks[i] = temp->task;
        temp = temp->next;
    }
    
    int remaining[count];
    for (int i = 0; i < count; i++) {
        remaining[i] = tasks[i]->burst;
    }
    
    printf("\nRound Robin (квант = %d)\n", QUANTUM);
    int done = 0;
    while (!done) {
        done = 1;
        for (int i = 0; i < count; i++) {
            if (remaining[i] > 0) {
                done = 0;
                if (remaining[i] > QUANTUM) {
                    run(tasks[i], QUANTUM);
                    remaining[i] -= QUANTUM;
                } else {
                    run(tasks[i], remaining[i]);
                    remaining[i] = 0;
                }
            }
        }
    }
    printf("\n\n");
    
    free(tasks);
}

// priority 
void priority_schedule(struct node *list) {
    int count = 0;
    struct node *temp = list;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    
    if (count == 0) return;
    
    Task **tasks = malloc(count * sizeof(Task*));
    temp = list;
    for (int i = 0; i < count; i++) {
        tasks[i] = temp->task;
        temp = temp->next;
    }
    
    // Сортировка по приоритету (от большего к меньшему)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (tasks[i]->priority < tasks[j]->priority) {
                Task *swap = tasks[i];
                tasks[i] = tasks[j];
                tasks[j] = swap;
            }
        }
    }
    
    printf("\nPriority Scheduling\n");
    for (int i = 0; i < count; i++) {
        run(tasks[i], tasks[i]->burst);
    }
    printf("Завершение Priority\n\n");
    
    free(tasks);
}

// priority + Round Robin планировщик
void priority_rr_schedule(struct node *list) {
    #define QUANTUM 10
    
    int count = 0;
    struct node *temp = list;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    
    if (count == 0) return;
    
    Task **tasks = malloc(count * sizeof(Task*));
    temp = list;
    for (int i = 0; i < count; i++) {
        tasks[i] = temp->task;
        temp = temp->next;
    }
    
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (tasks[i]->priority < tasks[j]->priority) {
                Task *swap = tasks[i];
                tasks[i] = tasks[j];
                tasks[j] = swap;
            }
        }
    }
    
    int remaining[count];
    for (int i = 0; i < count; i++) {
        remaining[i] = tasks[i]->burst;
    }
    
    printf("\nPriority + RR (квант = %d)\n", QUANTUM);
    int done = 0;
    while (!done) {
        done = 1;
        for (int i = 0; i < count; i++) {
            if (remaining[i] > 0) {
                int has_higher = 0;
                for (int j = 0; j < i; j++) {
                    if (remaining[j] > 0) {
                        has_higher = 1;
                        break;
                    }
                }
                
                if (!has_higher) {
                    done = 0;
                    if (remaining[i] > QUANTUM) {
                        run(tasks[i], QUANTUM);
                        remaining[i] -= QUANTUM;
                    } else {
                        run(tasks[i], remaining[i]);
                        remaining[i] = 0;
                    }
                }
            }
        }
    }
    printf("Завершение Priority + RR\n\n");
    
    free(tasks);
}


int load_tasks_from_file(const char *filename) {
    FILE *in;
    char line[256];
    char *temp;
    char *name;
    int priority, burst;
    
    in = fopen(filename, "r");
    if (in == NULL) {
        printf("\nне удалось открыть файл %s\n", filename);
        return 0;
    }
    
    while (fgets(line, sizeof(line), in) != NULL) {
        line[strcspn(line, "\n")] = 0;
        
        temp = strdup(line);
        name = strsep(&temp, ",");
        priority = atoi(strsep(&temp, ","));
        burst = atoi(strsep(&temp, ","));
        
        add_task(name, priority, burst);
        free(temp);
    }
    
    fclose(in);
    return 1;
}


void clear_tasks() {
    struct node *current = task_list;
    while (current != NULL) {
        struct node *next = current->next;
        free(current->task->name);
        free(current->task);
        free(current);
        current = next;
    }
    task_list = NULL;
}

// Главное меню
void show_menu() {

    printf("1 - показать все задачи\n");
    printf("2 - FCFS (First Come First Served)\n");
    printf("3 - SJF (Shortest Job First)\n");
    printf("4 -  Priority Scheduling\n");
    printf("5 - Round Robin\n");
    printf("6 - Priority + Round Robin\n");
    printf("0 -  выход\n");
    printf("выбрать - ");
}

int main() {
    int choice;
    
    
    if (load_tasks_from_file("schedule.txt")) {
        printf("Задачи загружены\n");
        display_tasks();
    } else {
        printf("Не удалось загрузить задачи.\n");
    }
    
    while (1) {
        show_menu();
        scanf("%d", &choice);
        getchar();
        
        switch (choice) {
            case 1:
                if (task_list == NULL) {
                    printf("\nНет загруженных задач!\n");
                } else {
                    display_tasks();
                }
                break;
                
            case 2:
                if (task_list == NULL) {
                    printf("\nНет загруженных задач!\n");
                } else {
                    fcfs_schedule(task_list);
                }
                break;
                
            case 3:
                if (task_list == NULL) {
                    printf("\nНет загруженных задач!\n");
                } else {
                    sjf_schedule(task_list);
                }
                break;
                
            case 4:
                if (task_list == NULL) {
                    printf("\nНет загруженных задач!\n");
                } else {
                    priority_schedule(task_list);
                }
                break;
                
            case 5:
                if (task_list == NULL) {
                    printf("\nНет загруженных задач!\n");
                } else {
                    rr_schedule(task_list);
                }
                break;
                
            case 6:
                if (task_list == NULL) {
                    printf("\nНет загруженных задач!\n");
                } else {
                    priority_rr_schedule(task_list);
                }
                break;
                
            case 0:
                clear_tasks();
                return 0;
                
            default:
                printf("\nНеверный выбор\n");
        }
    }
    
    return 0;
}