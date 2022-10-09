#include <iostream>
#include <locale>
#include <windows.h>

using namespace std;

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Russian");

	cout << "Название запускаемых процессов:" << endl;
	for (int i = 1; i < 5; i++) {
		cout << i << ") " << argv[i] << endl;
	}
	cout << endl;

	const char* fileNames[4] = {"1in.txt", "2out.txt", "3in.txt", "3out.txt"};

	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, true };
	HANDLE hFiles[4];
	hFiles[0] = CreateFileA(fileNames[0], GENERIC_READ, FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hFiles[1] = CreateFileA(fileNames[1], GENERIC_WRITE, FILE_SHARE_READ, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	hFiles[2] = CreateFileA(fileNames[2], GENERIC_READ, FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hFiles[3] = CreateFileA(fileNames[3], GENERIC_WRITE, FILE_SHARE_READ, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	for (int i = 0; i < 4; i++) {
		if (hFiles[i] == INVALID_HANDLE_VALUE) {
			cout << "Файл " << fileNames[i] << " не открывается!" << endl;
			CloseHandle(hFiles[0]);
			CloseHandle(hFiles[1]);
			CloseHandle(hFiles[2]);
			CloseHandle(hFiles[3]);
			return 0;
		}
	}

	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	STARTUPINFOA si[4];
	PROCESS_INFORMATION pi[4];
	for (int i = 0; i < 4; i++) {
		ZeroMemory(&si[i], sizeof(STARTUPINFOA));
		si[i].cb = sizeof(STARTUPINFOA);
		si[i].dwFlags = STARTF_USESTDHANDLES;
		ZeroMemory(&pi[i], sizeof(PROCESS_INFORMATION));
	}

	//Запуск первых трех процессов
	si[0].hStdInput = hFiles[0];
	si[0].hStdOutput = hStdOut;
	if (CreateProcessA(NULL, argv[1], NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si[0], &pi[0])) {
		cout << endl << endl << "Процесс 1 (" << argv[1] << ") запущен" << endl << endl;
	}
	else {
		cout << endl << endl << "Процесс 1 (" << argv[1] << ") не запустился" << endl << endl;
	}

	si[1].hStdInput = hStdIn;
	si[1].hStdOutput = hFiles[1];
	if (CreateProcessA(NULL, argv[2], NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si[1], &pi[1])) {
		cout << endl << endl << "Процесс 2 (" << argv[2] << ") запущен" << endl << endl;
	}
	else {
		cout << endl << endl << "Процесс 2 (" << argv[2] << ") не запустился" << endl << endl;
	}

	si[2].hStdInput = hFiles[2];
	si[2].hStdOutput = hFiles[3];
	if (CreateProcessA(NULL, argv[3], NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si[2], &pi[2])) {
		cout << endl << endl << "Процесс 3 (" << argv[3] << ") запущен" << endl << endl;
	}
	else {
		cout << endl << endl << "Процесс 3 (" << argv[3] << ") не запустился" << endl << endl;
	}
	cout << endl;

	HANDLE hProcesses[3];
	for (int i = 0; i < 3; i++) {
		hProcesses[i] = pi[i].hProcess;
	}
	WaitForMultipleObjects(3, hProcesses, TRUE, INFINITE);

	//Завершение первых трех процессов
	for (int i = 0; i < 3; i++) {
		CloseHandle(pi[i].hProcess);
		CloseHandle(pi[i].hThread);
		cout << endl << endl << "Процесс "<< i+1 << " завершен!" << endl << endl;
	}

	//Закрытие файлов
	for (int i = 0; i < 3; i++) {
		CloseHandle(hFiles[i]);
	}

	//Запуск последнего процесса
	si[3].hStdInput = hStdIn;
	si[3].hStdOutput = hStdOut;
	if (CreateProcessA(NULL, argv[4], NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si[3], &pi[3])) {
		cout << endl << endl << "Процесс 4 (" << argv[4] << ") запущен" << endl << endl;
	}
	else {
		cout << endl << endl << "Процесс 4 (" << argv[4] << ") не запустился" << endl << endl;
	}
	WaitForSingleObject(pi[3].hProcess, INFINITE);

	//Завершение последнего процесса
	CloseHandle(pi[3].hProcess);
	CloseHandle(pi[3].hThread);
	cout << endl << endl << "Процесс 4 завершен!" << endl << endl;

	return 0;
}