#include <windows.h>
#include <string>
#include <vector>

using namespace std;

//Количество аргументов для main (неявный аргумент),
//Количество строк для копирования,
//Номера строк[
// ...
// ...
// ]
//Имя исходного файла,
//Имя результирующего,
//Позиция вставки (begin/current/end).

//Пример аргументов
//4 1 5 7 8 file1.txt file2.txt begin


int main(int argc, char* argv[]) {
	int count_line = atoi(argv[1]);
	vector<int> number_line(count_line);
	for (int i = 0; i < count_line; i++) {
		number_line[i] = atoi(argv[2 + i]) - 1;
	}
	string initial_file_name = argv[count_line + 2];
	string result_file_name = argv[count_line + 3];
	string insert_position = argv[count_line + 4];

	//Преобразование string в LPCWSTR
	wstring ifn_wstr = wstring(initial_file_name.begin(), initial_file_name.end());
	LPCWSTR ifn_lpcwstr = ifn_wstr.c_str();
	wstring rfn_wstr = wstring(result_file_name.begin(), result_file_name.end());
	LPCWSTR rfn_lpcwstr = rfn_wstr.c_str();

	//Открытие исходного файла
	HANDLE file_1;
	file_1 = CreateFile(ifn_lpcwstr, GENERIC_READ,
		FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL);

	//Получение кол-ва символов в файле
	DWORD count_symbols = SetFilePointer(file_1, NULL, NULL, FILE_END);
	int size = (int)count_symbols;

	//Перемещение указателя в начало файла
	SetFilePointer(file_1, NULL, NULL, FILE_BEGIN);

	//Чтение файла и запись в vector строк
	LPVOID lpBuffer = new char*[size];
	DWORD lpNumberOfBytesRead;
	vector<string> lineS;
	string line = "";
	if (ReadFile(file_1, lpBuffer, count_symbols, &lpNumberOfBytesRead, NULL)) {
		for (int i = 0; i < size; i++) {
			if ((*((char*)lpBuffer + i) != '\n')) {
				line += *((char*)lpBuffer + i);
			}
			else {
				lineS.push_back(line);
				line = "";
			}
		}
		lineS.push_back(line);
	}

	//Закрытие исходного файла
	CloseHandle(file_1);

	//Открытие результирующего файла
	HANDLE file_2;
	SetFileAttributes(rfn_lpcwstr, FILE_ATTRIBUTE_NORMAL);
	file_2 = CreateFile(rfn_lpcwstr, GENERIC_WRITE,
		FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_ENCRYPTED, NULL);

	//Установка указателя в необходимую позицию файла
	if (insert_position == "begin") {
		SetFilePointer(file_2, NULL, NULL, FILE_BEGIN);
	}
	else if (insert_position == "end") {
		SetFilePointer(file_2, NULL, NULL, FILE_END);
	}
	else {
		int position = atoi(insert_position.c_str());
		SetFilePointer(file_2, position, NULL, FILE_BEGIN);
	}

	//Запись необходимых строк в результирующий файл
	for (int i = 0; i < count_line; i++) {
		DWORD lpNumberOfBytesWrite;
		string s = lineS[number_line[i]];
		const char* chars = s.c_str();
		WriteFile(file_2, chars, lineS[number_line[i]].size(), &lpNumberOfBytesWrite, NULL);
	}

	//Установка атрибутов для результирующего файла
	EncryptFile(rfn_lpcwstr);
	SetFileAttributes(rfn_lpcwstr, FILE_ATTRIBUTE_READONLY);

	//Закрытие результирующего файла
	CloseHandle(file_2);
	return 0;
}