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

//���������� �������
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

		//�������������� string � LPCWSTR
		wstring ifn_wstr = wstring(initial_file_name.begin(), initial_file_name.end());
		LPCWSTR ifn_lpcwstr = ifn_wstr.c_str();
		wstring rfn_wstr = wstring(result_file_name.begin(), result_file_name.end());
		LPCWSTR rfn_lpcwstr = rfn_wstr.c_str();

		//�������� ��������� �����
		HANDLE file_1;
		file_1 = CreateFile(ifn_lpcwstr, GENERIC_READ,
			FILE_SHARE_READ, NULL,
			OPEN_EXISTING, 0, NULL);

		//��������� ���-�� �������� � �����
		DWORD count_symbols = SetFilePointer(file_1, NULL, NULL, FILE_END);
		int size = (int)count_symbols;

		//����������� ��������� � ������ �����
		SetFilePointer(file_1, NULL, NULL, FILE_BEGIN);

		//������ ����� � ������ � vector �����
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

		//�������� ��������� �����
		CloseHandle(file_1);

		//�������� ��������������� �����
		HANDLE file_2;
		SetFileAttributes(rfn_lpcwstr, FILE_ATTRIBUTE_NORMAL);
		file_2 = CreateFile(rfn_lpcwstr, GENERIC_WRITE,
			FILE_SHARE_READ, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_ENCRYPTED, NULL);

		//��������� ��������� � ����������� ������� �����
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

		//������ ����������� ����� � �������������� ����
		for (int i = 0; i < count_line; i++) {
			DWORD lpNumberOfBytesWrite;
			string s = lineS[number_line[i]];
			const char* chars = s.c_str();
			WriteFile(file_2, chars, lineS[number_line[i]].size(), &lpNumberOfBytesWrite, NULL);
		}

		//��������� ��������� ��� ��������������� �����
		EncryptFile(rfn_lpcwstr);
		SetFileAttributes(rfn_lpcwstr, FILE_ATTRIBUTE_READONLY);

		//�������� ��������������� �����
		CloseHandle(file_2);
	}
	catch (const std::exception& e)
	{
		cout << "///////// Error. " << e.what() << '\n';
	}
}

//����� ������� ���������
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

	if (flag == "/i") { //������������� ���� � ������� �������
		si.hStdInput = hStdIn;
	}
	else if (flag == "/o") { //������������� ����� � ������� �������
		si.hStdOutput = hStdOut;
	}
	else if (flag == "/io") { //������������� ����/����� � �������
		si.hStdInput = hStdIn;
		si.hStdOutput = hStdOut;
	}
	else if (flag == "/f") { //������������� ����� � ����
		TCHAR szUser[MAX_PATH];
		SHGetSpecialFolderPath(NULL, szUser, CSIDL_DESKTOP, FALSE);
		string converted_szUser = ws2s(szUser) + "\\output_file.txt";
		//wstring path_desktop = wstring(converted_szUser.begin(), converted_szUser.end());
		LPCSTR path_desktop = &converted_szUser[0];

		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, true };
		HANDLE file = CreateFileA(path_desktop, GENERIC_WRITE, FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		si.hStdOutput = file;
		//������ ��������
		if (CreateProcessA(NULL, name_, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
			cout << endl << endl << "������� (" << name << ") �������" << endl << endl;
		}
		else {
			cout << endl << endl << "������� (" << name << ") �� ����������" << endl << endl;
		}
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		cout << endl << endl << "������� ��������!" << endl << endl;
		CloseHandle(file);

		si.hStdInput = hStdIn;
		si.hStdOutput = hStdOut;

		return;
	}

	//������ ��������
	if (CreateProcessA(NULL, name_, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		cout << endl << endl << "������� (" << name << ") �������" << endl << endl;
	}
	else {
		cout << endl << endl << "������� (" << name << ") �� ����������" << endl << endl;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	cout << endl << "������� ��������!" << endl << endl;
	return;
}

int main() {
	setlocale(LC_ALL, "Russian");

	string command;
	while (true) {
		cout << "$: ";
		getline(cin, command);
		vector<string> command_substrings = CommandParsing(command, " ");
		if (command_substrings.size() == 0) continue; //���� ������ �� �������
		if (command_substrings[0] == "execute") { //���� ������� ����������� �������
			if (command_substrings.size() > 3) { //���� ���������� ����� 2 ���������� (�� ������ �������� �������)
				cout << "///////// Error. Too many arguments." << endl;
				continue;
			}
			if (command_substrings.size() > 1) { //�������� �� ������� ����������
				if (command_substrings.size() == 3) { //���� ������������ ����������� �������
					//������� ������� ������� � ������
					Execute(command_substrings[1], command_substrings[2]);
					continue;
				}
				else {
					//������� ������� ������� ��� ������
					Execute(command_substrings[1]);
					continue;
				}
			}
			else { //���� ������� ���� ����������
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
		else { //���� ������� ����������� �������
			cout << "///////// Invalid command." << endl; 
			continue;
		}
	}
}