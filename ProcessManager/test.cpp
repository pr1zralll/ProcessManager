#include "Log.h"
#include "Process.h"
#include <iostream>
using namespace std;

int main()
{
	Log::Clear();
	Process *cat = new Process(L"C:\\Users\\Pasha\\Desktop\\SP_lab_1.exe",L"");
	cat->start();         
	cout<<cat->getState();  
	//cout << Log::getPath();
	system("start program.log");
	system("pause>nul");
}