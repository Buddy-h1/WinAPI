#include <iostream>
#include <windows.h>
#include <shlobj_core.h>
#include <codecvt>
#include <string>
#include <vector>

using namespace std;

vector<string> CommandParsing(string str, string separator) {
	vector<string> command_substrings;
	size_t pos = 0;
	while ((pos = str.find(separator)) != -1) {
		command_substrings.push_back(str.substr(0, pos));
		str.erase(0, pos + separator.length());
	}
	command_substrings.push_back(str.substr(0, str.length()));
	return command_substrings;
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

//Встроенная команда
void CopyStrings(vector<string> &argv) {
	try
	{
		int count_line = stoi(argv.at(0));
		vector<int> number_line(count_line);
		for (int i = 0; i < count_line; i++) {
			number_line[i] = stoi(argv.at(1 + i));
		}
		string initial_file_name = argv.at(count_line + 1);
		string result_file_name = argv.at(count_line + 2);
		string insert_position = argv.at(count_line + 3);

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
		LPVOID lpBuffer = new char* [size];
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
	}
	catch (const std::exception& e)
	{
		cout << "///////// Error. " << e.what() << '\n';
	}
}

//Вызов внешней программы
void Execute(string name, string flag = "") {

	LPSTR name_ = (LPSTR)name.c_str();

	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFOA));
	si.cb = sizeof(STARTUPINFOA);
	si.dwFlags = STARTF_USESTDHANDLES;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	if (flag == "/i") { //Перенаправить ввод в текущую консоль
		si.hStdInput = hStdIn;
	}
	else if (flag == "/o") { //Перенаправить вывод в текущую консоль
		si.hStdOutput = hStdOut;
	}
	else if (flag == "/io") { //Перенаправить ввод/вывод в консоль
		si.hStdInput = hStdIn;
		si.hStdOutput = hStdOut;
	}
	else if (flag == "/f") { //Перенаправить вывод в файл
		TCHAR szUser[MAX_PATH];
		SHGetSpecialFolderPath(NULL, szUser, CSIDL_DESKTOP, FALSE);
		string converted_szUser = ws2s(szUser) + "\\output_file.txt";
		//wstring path_desktop = wstring(converted_szUser.begin(), converted_szUser.end());
		LPCSTR path_desktop = &converted_szUser[0];

		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, true };
		HANDLE file = CreateFileA(path_desktop, GENERIC_WRITE, FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		si.hStdOutput = file;
		//Запуск процесса
		if (CreateProcessA(NULL, name_, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
			cout << endl << endl << "Процесс (" << name << ") запущен" << endl << endl;
		}
		else {
			cout << endl << endl << "Процесс (" << name << ") не запустился" << endl << endl;
		}
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		cout << endl << endl << "Процесс завершен!" << endl << endl;
		CloseHandle(file);

		si.hStdInput = hStdIn;
		si.hStdOutput = hStdOut;

		return;
	}

	//Запуск процесса
	if (CreateProcessA(NULL, name_, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		cout << endl << endl << "Процесс (" << name << ") запущен" << endl << endl;
	}
	else {
		cout << endl << endl << "Процесс (" << name << ") не запустился" << endl << endl;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	cout << endl << "Процесс завершен!" << endl << endl;
	return;
}

int main() {
	setlocale(LC_ALL, "Russian");

	string command;
	while (true) {
		cout << "$: ";
		getline(cin, command);
		vector<string> command_substrings = CommandParsing(command, " ");
		if (command_substrings.size() == 0) continue; //Если ничего не введено
		if (command_substrings[0] == "execute") { //Если вызвана действующая команда
			if (command_substrings.size() > 3) { //Если содержится более 2 аргументов (не считая название команды)
				cout << "///////// Error. Too many arguments." << endl;
				continue;
			}
			if (command_substrings.size() > 1) { //Проверка на наличие аргументов
				if (command_substrings.size() == 3) { //Если используются управляющие символы
					//Вызвать внешний процесс с флагом
					Execute(command_substrings[1], command_substrings[2]);
					continue;
				}
				else {
					//Вызвать внешний процесс БЕЗ фалага
					Execute(command_substrings[1]);
					continue;
				}
			}
			else { //Если слишком мало аргументов
				cout << "///////// Error. Too few arguments." << endl;
				continue;
			}
		}
		else if (command_substrings[0] == "copy_strings") {
			string temp = command;
			temp.erase(0, 13);
			vector<string> arguments = CommandParsing(temp, " ");	
			CopyStrings(arguments);
		}
		else if (command_substrings[0] == "quit" && command_substrings.size() == 1) {
			return 0;
		}
		else { //Если вызвана неизвестная команда
			cout << "///////// Invalid command." << endl; 
			continue;
		}
	}
}