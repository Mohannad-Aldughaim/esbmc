// QVector::begin
#include <iostream>
#include <QVector>
#include <cassert>
using namespace std;

int main ()
{
  int myints[] = {75,23,65,42,13};
  QVector<int> myQVector;
  for(int i = 0; i < 5; i++)
    myQVector.push_back(myints[i]);
  QVector<int>::const_iterator it;

  it = myQVector.constBegin();
  assert(*it != 75);
  
  cout << endl;

  return 0;
}
