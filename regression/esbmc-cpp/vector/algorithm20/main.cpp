// replace_if example
#include <iostream>
#include <cassert>
#include <vector>
using namespace std;

template<class FwdIt, class Pr, class Ty>
void replace_if(FwdIt first, FwdIt last, Pr pred, const Ty& val) {
	for (; first != last; ++first)
		if (pred(*first))
			*first = val;
}

bool IsOdd (int i) { return ((i%2)==1); }

int main () {
  vector<int> myvector;
  vector<int>::iterator it;

  // set some values:
  for (int i=1; i<10; i++) myvector.push_back(i);          // 1 2 3 4 5 6 7 8 9
  
  replace_if (myvector.begin(), myvector.end(), IsOdd, 0); // 0 2 0 4 0 6 0 8 0
  assert(myvector[0] == 0);
  assert(myvector[4] == 0);
  cout << "myvector contains:";
  for (it=myvector.begin(); it!=myvector.end(); ++it)
    cout << " " << *it;

  cout << endl;
 
  return 0;
}
