//#include <QtCore>
#include <QList>
#include <iostream>
using namespace std;

int main()
{
    QList<int>* list = new QList<int>();

    list->push_back(0);
    list->push_back(1);
    list->push_back(2);
    list->push_back(4);
    
    
    list->indexOf(3) == -1 ?
	   std::cout<<"No Element has been found!"<<std::endl
	  :std::cout<<"Element found at index "<<list->indexOf(3)<<std::endl; 

	assert(list->back() == 5);
	return 0;
}
