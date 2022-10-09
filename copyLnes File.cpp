#include <windows.h>
#include <string>
#include <vector>

using namespace std;

//���������� ���������� ��� main (������� ��������),
//���������� ����� ��� �����������,
//������ �����[
// ...
// ...
// ]
//��� ��������� �����,
//��� ���������������,
//������� ������� (begin/current/end).

//������ ����������
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
	return 0;
}