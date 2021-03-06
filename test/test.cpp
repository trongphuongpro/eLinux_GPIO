#include <unistd.h>
#include <iostream>
#include "gpio.h"

using namespace BBB;
using namespace std;


int resetAllLeds(void*);
int ledNumber = 5;

int main() {

	GPIO *pin1 = new GPIO(P8_46, GPIO::OUTPUT);
	GPIO *pin2 = new GPIO(P8_44, GPIO::OUTPUT);
	GPIO *pin3 = new GPIO(P8_42, GPIO::OUTPUT);
	GPIO *pin4 = new GPIO(P8_40, GPIO::OUTPUT);
	GPIO *pin5 = new GPIO(P8_38, GPIO::OUTPUT);
	GPIO *pin6 = new GPIO(P8_36, GPIO::INPUT);

	GPIO* container[ledNumber] = {pin1, pin2, pin3, pin4, pin5};

	pin6->setInterruptMode(GPIO::RISING);
	pin6->setDebounceTime(10);
	pin6->onInterrupt(resetAllLeds, container);


	pin1->toggle(1);
	pin2->toggle(2);
	pin3->toggle(4);
	pin4->toggle(8);
	pin5->toggle(16);

	while (1) {

	}
}


int resetAllLeds(void* arg) {
	GPIO** led = (GPIO**)arg;

	static int count = 0;
	count++;

	cout << count << endl;

	for (int i = 0; i < ledNumber; i++) {
		if (count % 2 == 1) {
			led[i]->stopToggle();
		}
		else {
			led[i]->toggle(5);
		}
	}
	return 0;
}