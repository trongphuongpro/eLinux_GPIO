#include <unistd.h>
#include <iostream>
#include "gpio.h"

using namespace BBB;
using namespace std;


int resetAllLeds();

GPIO pin1(67, OUTPUT);
GPIO pin2(68, OUTPUT);
GPIO pin3(44, OUTPUT);
GPIO pin4(26, OUTPUT);
GPIO pin5(46, OUTPUT);
GPIO pin6(66, INPUT);

int main() {

	pin6.setInterruptEdge(RISING);
	pin6.setDebounceTime(5);
	pin6.waitEdge(resetAllLeds);

	pin1.toggle(1);
	pin2.toggle(2);
	pin3.toggle(4);
	pin4.toggle(8);
	pin5.toggle(16);

	while (1) {

	}
}


int resetAllLeds() {
	static int count = 0;
	count++;

	cout << count << endl;

	if (count % 2 == 1) {
		pin1.stopToggle();
		pin2.stopToggle();
		pin3.stopToggle();
		pin4.stopToggle();
		pin5.stopToggle();
	}
	else {
		pin1.toggle(1);
		pin2.toggle(2);
		pin3.toggle(4);
		pin4.toggle(8);
		pin5.toggle(16);
	}

	return 0;
}