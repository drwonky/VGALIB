// compiler test for old Borland C++
#ifdef __GNUC__
#include <iostream>
 
using namespace std;
#else
#include <iostream.h>
#endif


class Base
{
public:
    int x;
		Base() { x=0; }
		Base( int i ) : Base() { x=i; }
};

int main()
{
	Base b1;
	Base b2(42);

	cout<<b1.x<<endl;
	cout<<b2.x<<endl;
    
    return 0;
}
