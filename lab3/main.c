#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define PAGE_SIZE 256           
#define PAGE_TABLE_SIZE 256     
#define TLB_SIZE 16             
#define FRAME_COUNT 256         
#define MEMORY_SIZE (FRAME_COUNT * PAGE_SIZE) 
#define BACKING_STORE "BACKING_STORE.bin"
#define ADDRESSES_FILE "addresses.txt"

#define PAGE_NUMBER_MASK 0xFF00
#define OFFSET_MASK       0x00FF
#define PAGE_SHIFT        8

typedef struct {
    int page_number;
    int frame_number;
    int valid;
    int lru_counter; 
} TLBEntry;


TLBEntry tlb[TLB_SIZE];
int page_table[PAGE_TABLE_SIZE];
char physical_memory[MEMORY_SIZE];
int free_frame_index = 0;
int lru_clock = 0;


int total_addresses = 0;
int page_faults = 0;
int tlb_hits = 0;


int backing_fd;
void init_system();
int get_physical_address(int logical_address);
int search_tlb(int page_number);
void update_tlb(int page_number, int frame_number);
int handle_page_fault(int page_number);
char read_byte(int physical_address);
void print_statistics();

int main() {
    FILE *addr_file = fopen(ADDRESSES_FILE, "r");
    if (!addr_file) {
        fprintf(stderr, "Error: Cannot open %s\n", ADDRESSES_FILE);
        fprintf(stderr, "Make sure %s is in the same directory as the executable.\n", ADDRESSES_FILE);
        return 1;
    }
    backing_fd = open(BACKING_STORE, O_RDONLY);
    if (backing_fd < 0) {
        perror("Failed to open BACKING_STORE.bin");
        fprintf(stderr, "Make sure %s is in the same directory as the executable.\n", BACKING_STORE);
        fclose(addr_file);
        return 1;
    }
    init_system();

    printf("=== Virtual Memory Manager ===\n");
    printf("Processing addresses from %s...\n\n", ADDRESSES_FILE);

    int logical_addr;
    while (fscanf(addr_file, "%d", &logical_addr) == 1) {
        total_addresses++;
        logical_addr &= 0xFFFF;    
        int physical_addr = get_physical_address(logical_addr);
        char value = read_byte(physical_addr);  
        printf("Logical: %5d -> Physical: %5d -> Value: %d\n", 
               logical_addr, physical_addr, value);
    }

    print_statistics();
    fclose(addr_file);
    close(backing_fd); 
    return 0;
}

void init_system() {
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        page_table[i] = -1;
    }
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].valid = 0;
        tlb[i].lru_counter = 0;
    }
    
    
    memset(physical_memory, 0, MEMORY_SIZE);
}

int search_tlb(int page_number) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page_number == page_number) {
            tlb[i].lru_counter = lru_clock++;
            return tlb[i].frame_number;
        }
    }
    return -1; 
}

void update_tlb(int page_number, int frame_number) {
    
    int free_slot = -1;
    for (int i = 0; i < TLB_SIZE; i++) {
        if (!tlb[i].valid) {
            free_slot = i;
            break;
        }
    }
    
    
    if (free_slot == -1) {
        int lru_index = 0;
        int min_counter = tlb[0].lru_counter;
        for (int i = 1; i < TLB_SIZE; i++) {
            if (tlb[i].lru_counter < min_counter) {
                min_counter = tlb[i].lru_counter;
                lru_index = i;
            }
        }
        free_slot = lru_index;
    }
    
    
    tlb[free_slot].page_number = page_number;
    tlb[free_slot].frame_number = frame_number;
    tlb[free_slot].valid = 1;
    tlb[free_slot].lru_counter = lru_clock++;
}

int handle_page_fault(int page_number) {
    if (free_frame_index >= FRAME_COUNT) {
        fprintf(stderr, "Error: Physical memory full (should not happen with 256 frames)\n");
        exit(1);
    }
    
    off_t offset = page_number * PAGE_SIZE;
    
    
    if (lseek(backing_fd, offset, SEEK_SET) < 0) {
        perror("lseek error");
        exit(1);
    }
    
    
    char *frame_ptr = &physical_memory[free_frame_index * PAGE_SIZE];
    ssize_t bytes_read = read(backing_fd, frame_ptr, PAGE_SIZE);
    if (bytes_read != PAGE_SIZE) {
        fprintf(stderr, "Error reading page %d from backing store\n", page_number);
        exit(1);
    }
    page_table[page_number] = free_frame_index;
    
    
    int allocated_frame = free_frame_index;
    free_frame_index++;
    
    page_faults++;
    return allocated_frame;
}

int get_physical_address(int logical_address) {
    
    int page_number = (logical_address & PAGE_NUMBER_MASK) >> PAGE_SHIFT;
    int offset = logical_address & OFFSET_MASK;
    
    
    int frame_number = search_tlb(page_number);
    
    if (frame_number != -1) {
        
        tlb_hits++;
    } else {
        
        frame_number = page_table[page_number];
        
        if (frame_number == -1) {
            
            frame_number = handle_page_fault(page_number);
        }
        
        
        update_tlb(page_number, frame_number);
    }
    
    
    return (frame_number * PAGE_SIZE) + offset;
}

char read_byte(int physical_address) {

    return physical_memory[physical_address];
}

void print_statistics() {
    printf("\n=== Статистика ===\n");
    printf("Всего обработано: %d\n", total_addresses);
    printf("ошибок страниц: %d\n", page_faults);
    printf("частота ошбок страниц: %.2f%%\n", 
           (float)page_faults / total_addresses * 100.0);
    printf("частота TLB: %d\n", tlb_hits);
    printf("Частота попаданий в TLB: %.2f%%\n", 
           (float)tlb_hits / total_addresses * 100.0);
}