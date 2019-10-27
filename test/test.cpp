#include <unistd.h>
#include <iostream>
#include "gpio.h"
#include <vector>

using namespace BBB;
using namespace std;


int resetAllLeds(void*);
int ledNumber = 5;

int main() {

	GPIO *pin1 = new GPIO(67, OUTPUT);
	GPIO *pin2 = new GPIO(68, OUTPUT);
	GPIO *pin3 = new GPIO(44, OUTPUT);
	GPIO *pin4 = new GPIO(26, OUTPUT);
	GPIO *pin5 = new GPIO(46, OUTPUT);
	GPIO *pin6 = new GPIO(66, INPUT);

	//vector<GPIO *> container;
	// container.push_back(pin1);
	// container.push_back(pin2);
	// container.push_back(pin3);
	// container.push_back(pin4);
	// container.push_back(pin5);

	GPIO* container[ledNumber] = {pin1, pin2, pin3, pin4, pin5};

	//cout << "[main] address: " << container.data() << endl;
	//cout << "[main] void*: " << static_cast<void*>(container.data()) << endl;

	pin6->setInterruptEdge(RISING);
	pin6->setDebounceTime(5);
	pin6->waitEdge(resetAllLeds, (void*)(container));

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

	cout << "[resetAllLeds] void*: " << arg << endl;
	cout << "[resetAllLeds] address: " << led << endl;

	static int count = 0;
	count++;

	cout << count << endl;

	for (int i = 0; i < ledNumber; i++) {
		if (count % 2 == 1) {
			led[i]->stopToggle();
		}
		else {
			led[i]->toggle(2);
		}
	}
	return 0;
}