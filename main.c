#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <windows.h>
#include <semaphore.h>

// Sabit degerler
#define FLOOR_COUNT 10
#define APARTMENT_COUNT 4

// Her katin durumunu takip eden yapi
typedef struct {
    int floor_number;
    int is_completed;
    pthread_mutex_t floor_mutex;
    sem_t floor_ready;
} Floor;

// Her dairenin bilgilerini tutan yapi
typedef struct {
    int floor_number;
    int apartment_number;
} Apartment;

// Global degiskenler
Floor building_floors[FLOOR_COUNT];
pthread_mutex_t crane_mutex;    // Vinc erisimi icin mutex
pthread_mutex_t painter_mutex;  // Boyaci ekibi erisimi icin mutex

// Daire insaati thread fonksiyonu
// Her daire kendi insaat islemlerini yapar ve vinci kullanir
void* construct_apartment(void* arg) {
    Apartment* apartment = (Apartment*)arg;
    
    printf("Kat %d, Daire %d insaata basladi\n", apartment->floor_number + 1, apartment->apartment_number + 1);
    
    // Vinc kullanimi icin mutex kilidi
    // Ayni anda sadece bir daire vinc kullanabilir
    pthread_mutex_lock(&crane_mutex);
    printf("Kat %d, Daire %d vinc kullaniyor\n", apartment->floor_number + 1, apartment->apartment_number + 1);
    Sleep(500); // Vinc kullanim simulasyonu - 1 saniye
    pthread_mutex_unlock(&crane_mutex);
    
    // Daire insaat islemleri simulasyonu
    // Temel islemler: duvar orme, tesisat, siva vb.
    Sleep(500); // Insaat islemi simulasyonu - 2 saniye

    // Boyaci ekibi kullanimi icin mutex kilidi
    // Ayni anda sadece bir daire boyaci ekibini kullanabilir
    pthread_mutex_lock(&painter_mutex);
    printf("Kat %d, Daire %d boya islemi yapiliyor\n", apartment->floor_number + 1, apartment->apartment_number + 1);
    Sleep(500); // Boya islemi simulasyonu - 1.5 saniye
    printf("Kat %d, Daire %d boya islemi tamamlandi\n", apartment->floor_number + 1, apartment->apartment_number + 1);
    pthread_mutex_unlock(&painter_mutex);
    
    printf("Kat %d, Daire %d tamamlandi\n", apartment->floor_number + 1, apartment->apartment_number + 1);
    free(apartment);
    return NULL;
}

// Kat insaati thread fonksiyonu
// Her kat kendi dairelerinin insaatini yonetir
void* construct_floor(void* arg) {
    int floor_number = *((int*)arg);
    pthread_t apartment_threads[APARTMENT_COUNT];
    
    // Alt katin tamamlanmasini bekle
    // Ilk kat haric tum katlar kendinden onceki katin tamamlanmasini bekler
    if (floor_number > 0) {
        sem_wait(&building_floors[floor_number-1].floor_ready);
    }
    
    printf("Kat %d insaati basladi\n", floor_number + 1);
    
    // Kattaki tum dairelerin insaatini baslat
    for (int i = 0; i < APARTMENT_COUNT; i++) {
        Apartment* new_apartment = malloc(sizeof(Apartment));
        new_apartment->floor_number = floor_number;
        new_apartment->apartment_number = i;
        pthread_create(&apartment_threads[i], NULL, construct_apartment, new_apartment);
    }
    
    // Kattaki tum dairelerin tamamlanmasini bekle
    for (int i = 0; i < APARTMENT_COUNT; i++) {
        pthread_join(apartment_threads[i], NULL);
    }
    
    printf("Kat %d tamamen tamamlandi\n", floor_number + 1);
    building_floors[floor_number].is_completed = 1;
    
    // Bir sonraki kata baslamasi icin sinyal gonder
    sem_post(&building_floors[floor_number].floor_ready);
    
    free(arg);
    return NULL;
}

int main() {
    pthread_t floor_threads[FLOOR_COUNT];
    
    // Mutex ve semaforlari baslat
    pthread_mutex_init(&crane_mutex, NULL);
    pthread_mutex_init(&painter_mutex, NULL);  // Yeni boyaci mutex'i baslatiliyor
    for (int i = 0; i < FLOOR_COUNT; i++) {
        building_floors[i].floor_number = i;
        building_floors[i].is_completed = 0;
        pthread_mutex_init(&building_floors[i].floor_mutex, NULL);
        sem_init(&building_floors[i].floor_ready, 0, 0);
    }
    
    printf("Bina insaati basliyor...\n");
    
    // Tum katlarin insaatini baslat
    for (int i = 0; i < FLOOR_COUNT; i++) {
        int* current_floor = malloc(sizeof(int));
        *current_floor = i;
        pthread_create(&floor_threads[i], NULL, construct_floor, current_floor);
    }
    
    // Tum katlarin tamamlanmasini bekle
    for (int i = 0; i < FLOOR_COUNT; i++) {
        pthread_join(floor_threads[i], NULL);
    }
    
    printf("Bina insaati tamamlandi!\n");
    
    // Kullanilan kaynaklari temizle
    pthread_mutex_destroy(&crane_mutex);
    pthread_mutex_destroy(&painter_mutex);  // Boyaci mutex'i temizleniyor
    for (int i = 0; i < FLOOR_COUNT; i++) {
        pthread_mutex_destroy(&building_floors[i].floor_mutex);
        sem_destroy(&building_floors[i].floor_ready);
    }
    
    return 0;
} 