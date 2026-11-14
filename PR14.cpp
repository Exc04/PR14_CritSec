// PR14.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>


using namespace std;

#define MAX_ZAPISEY 1000
#define MAX_TEKSTA 128

struct ZaprosLoga {
    DWORD idPotoka;
    int prioritet;
    DWORD tiki;
    char soobshenie[MAX_TEKSTA];
};

struct BuferLogov {
    ZaprosLoga zapisi[MAX_ZAPISEY];
    LONG index;
};

BuferLogov bufer_logov;
CRITICAL_SECTION cs;
bool stop_programm = false;
int isp_cs = 0;

void rand_zaderzhka() {
    int ms = 10 + rand() % 91;
    Sleep(ms);
}

DWORD WINAPI PotokLogera(LPVOID param) {
    int prioritet_potoka = (int)(intptr_t)param;

    while (!stop_programm)
    {
        ZaprosLoga new_zapis;
        new_zapis.idPotoka = GetCurrentThreadId();
        new_zapis.prioritet = prioritet_potoka;
        new_zapis.tiki = GetTickCount();

        if (prioritet_potoka == THREAD_PRIORITY_HIGHEST)
            strcpy(new_zapis.soobshenie, "ВЫСОКИЙ");
        else if (prioritet_potoka == THREAD_PRIORITY_NORMAL)
            strcpy(new_zapis.soobshenie, "СРЕДНИЙ");
        else strcpy(new_zapis.soobshenie, "НИЗКИЙ");

        if (isp_cs == 1) {
            EnterCriticalSection(&cs);
        }

        if (bufer_logov.index < MAX_ZAPISEY) {
            bufer_logov.zapisi[bufer_logov.index] = new_zapis;
            bufer_logov.index++;
        }

        if (isp_cs == 1) {
            LeaveCriticalSection(&cs);
        }

        rand_zaderzhka();
    }
    return 0;
}

DWORD WINAPI PotokNablyudatelya(LPVOID param) {
    while (!stop_programm) {
        if (isp_cs == 1) {
            EnterCriticalSection(&cs); 
        }

        cout << "Последние 5 записей: " << endl;
        LONG tekushiy_index = bufer_logov.index;

        if (tekushiy_index > 0) {
            int nachalo = tekushiy_index - 5;
            if (nachalo < 0) nachalo = 0;

            for (int i = nachalo; i < tekushiy_index; i++) {
                cout << "[" << i << "] Поток: " <<
                    bufer_logov.zapisi[i].idPotoka << " | Приоритет: "
                    << bufer_logov.zapisi->prioritet << " | Тики: " <<
                    bufer_logov.zapisi[i].tiki << " | Сообщение: " <<
                    bufer_logov.zapisi[i].soobshenie << endl;
            }
        }
        else {
            cout << "Буфер пуст" << endl;
        }

        if (isp_cs == 1) {
            LeaveCriticalSection(&cs);
        }

        cout << "----------------\n" << endl;

        Sleep(100);
    }
    return 0;
}

int main()
{
    srand(time(NULL));
    setlocale(NULL, "Russian");
    
    SetProcessAffinityMask(GetCurrentProcess(), 1);
    cout << "Выберите режим:" << endl;
    cout << "1 - С крит. секциями" << endl;
    cout << "2 - Без крит. секций" << endl;
    cout << "Ваш выбор: ";
    cin >> isp_cs;
    if (isp_cs != 1)
        isp_cs = 0;

    bufer_logov.index = 0;

    if (isp_cs == 1) {
        InitializeCriticalSection(&cs);
    }

    HANDLE t1 = CreateThread(NULL, 0, PotokLogera, (LPVOID)(intptr_t)THREAD_PRIORITY_HIGHEST, 0, NULL);
    HANDLE t2 = CreateThread(NULL, 0, PotokLogera, (LPVOID)(intptr_t)THREAD_PRIORITY_NORMAL, 0, NULL);
    HANDLE t3 = CreateThread(NULL, 0, PotokLogera, (LPVOID)(intptr_t)THREAD_PRIORITY_BELOW_NORMAL, 0, NULL);
    HANDLE t4 = CreateThread(NULL, 0, PotokNablyudatelya, NULL, 0, NULL);

    SetThreadPriority(t1, THREAD_PRIORITY_HIGHEST);
    SetThreadPriority(t2, THREAD_PRIORITY_NORMAL);
    SetThreadPriority(t3, THREAD_PRIORITY_BELOW_NORMAL);
    SetThreadPriority(t4, NULL);

    Sleep(5000);

    stop_programm = true;

    WaitForSingleObject(t1, INFINITE);
    WaitForSingleObject(t2, INFINITE);
    WaitForSingleObject(t3, INFINITE);
    WaitForSingleObject(t4, INFINITE);

    CloseHandle(t1);
    CloseHandle(t2);
    CloseHandle(t3);
    CloseHandle(t4);

    if (isp_cs == 1) {
        DeleteCriticalSection(&cs);
    }

    cout << "Программа завершена. Всего записей: " << bufer_logov.index << endl;

    return 0;
}

