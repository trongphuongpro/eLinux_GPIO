#include "gpio.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <pthread.h>


using namespace std;

namespace BBB {

GPIO::GPIO(int pin, GPIO_DIRECTION mode) {
	this->pin = pin;
	this->debounceTime = 0;
	this->toggleFrequency = 0;
	this->toggleNumber = 0;
	this->callbackFunction = NULL;
	this->threadRunning = false;
	this->interruptEdge = NONE;

	ostringstream s;
	s << "gpio" << pin;
	cout << s.str() << " created" << endl;

	this->name = string(s.str());
	this->path = GPIO_PATH + this->name + "/";

	setPinMode(mode);
	
	usleep(250000);
}


GPIO::~GPIO() {
	cout << "Object destructed" << endl;
}


int GPIO::getPin() {
	return this->pin;
}


void GPIO::setDebounceTime(int time) {
	this->debounceTime = time;
}


int GPIO::write(string path, string filename, string value) {
	ofstream fs;
	fs.open((path + filename).c_str());
	if (!fs.is_open()) {
		perror("GPIO: write failed to open file ");
		return -1;
	}

	fs << value;
	fs.close();
	return 0;
}


int GPIO::write(string path, string filename, int value){
   stringstream s;
   s << value;
   return this->write(path, filename, s.str());
}


string GPIO::read(string path, string filename) {
	ifstream fs;
	fs.open((path + filename).c_str());
	if (!fs.is_open()) {
		perror("GPIO: read failed to open file ");
	}
	string input;
	getline(fs, input);
	fs.close();
	return input;
}


int GPIO::setPinMode(GPIO_DIRECTION mode) {
	if (mode == INPUT) {
		return this->write(this->path, "direction", "in");
	}
	if (mode == OUTPUT) {
		return this->write(this->path, "direction", "out");
	}
	return -1;
}


GPIO_DIRECTION GPIO::getPinMode() {
	string input = this->read(this->path, "direction");
	if (input == "in")
		return INPUT;
	else
		return OUTPUT;
}


int GPIO::digitalWrite(GPIO_VALUE value) {
	if (value == LOW) {
		return this->write(this->path, "value", "0");
	}
	if (value == HIGH) {
		return this->write(this->path, "value", "1");
	}
	return -1;
}


GPIO_VALUE GPIO::digitalRead() {
	string input = this->read(this->path, "value");
	if (input == "0")
		return LOW;
	else
		return HIGH;
}


void GPIO::setInterruptEdge(GPIO_EDGE type) {
	this->interruptEdge = type;
	this->setEdgeType(type);
}


int GPIO::setEdgeType(GPIO_EDGE type) {
	if (type == NONE) {
		return this->write(this->path, "edge", "none");
	}
	if (type == RISING) {
		return this->write(this->path, "edge", "rising");
	}
	if (type == FALLING) {
		return this->write(this->path, "edge", "falling");
	}
	if (type == BOTH) {
		return this->write(this->path, "edge", "both");
	}
	return -1;
}


GPIO_EDGE GPIO::getEdgeType() {
	string input = this->read(this->path, "edge");
	if (input == "rising") 
		return RISING;
	else if (input == "falling") 
		return FALLING;
	else if (input == "both") 
		return BOTH;
	else 
		return NONE;
}


int GPIO::setActiveMode(GPIO_VALUE mode) {
	if (mode == LOW) {
		return this->write(this->path, "active_low", "1");
	}
	else if (mode == HIGH) {
		return this->write(this->path, "active_low", "0");
	}
	return 0;
}


int GPIO::openStream() {
	stream.open((this->path + "value").c_str());
	return 0;
}


int GPIO::writeStream(GPIO_VALUE value) {
	stream << value << flush;
	return 0;
}


int GPIO::closeStream() {
	stream.close();
	return 0;
}


int GPIO::toggle() {
	if (getPinMode() == INPUT) {
		perror("GPIO: Cannot toggle input pin");
		return -1;
	}
	
	if (digitalRead() == LOW)
		digitalWrite(HIGH);
	else
		digitalWrite(LOW);

	return 0;
}


int GPIO::toggle(int frequency) {
	return toggle(-1, frequency);
}


int GPIO::toggle(int numberOfTimes, int frequency) {
	if (getPinMode() == INPUT) {
		perror("GPIO: Cannot toggle input pin");
		return -1;
	}
	
	toggleNumber = numberOfTimes;
	toggleFrequency = frequency;
	threadRunning = true;

	if (pthread_create(&this->thread, 
						NULL, 
						threadedToggle, 
						static_cast<void*>(this))) {
		
		perror("GPIO: Failed to create the toggle thread");
		threadRunning = false;
		return -1;
	}
	
	return 0;
}


void *threadedToggle(void *value) {
	GPIO *gpio = static_cast<GPIO*>(value);
	bool isHigh = (bool)gpio->digitalRead();

	while (gpio->threadRunning) {
		if (isHigh) {
			gpio->digitalWrite(LOW);
		}
		else {
			gpio->digitalWrite(HIGH);
		}
		usleep(500000 / gpio->toggleFrequency);
		isHigh = !isHigh;

		if (gpio->toggleNumber > 0) {
			gpio->toggleNumber--;
		}
		else if (gpio->toggleNumber == 0) {
			gpio->threadRunning = false;
		}
	}
	return 0;
}


void GPIO::changeToggleFrequency(int frequency) {
	this->toggleFrequency = frequency;
}


void GPIO::stopToggle() {
	this->threadRunning = false;
}


int GPIO::waitEdge() {
	if (getPinMode() == OUTPUT) {
		perror("GPIO: Cannot wait edge on output pin");
		return -1;
	}

	int fd, i, epollfd, count = 0;
	struct epoll_event ev;
	epollfd = epoll_create(1);

	if (epollfd == -1) {
		perror("GPIO: Failed to create epollfd");
		return -1;
	}

	if ((fd = open((this->path + "value").c_str(), O_RDONLY | O_NONBLOCK)) == -1) {
		perror("GPIO: Failed to open file");
    	return -1;
	}

	ev.events = EPOLLIN | EPOLLET | EPOLLPRI;
	ev.data.fd = fd;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("GPIO: Failed to add control interface");
       	return -1;
	}

	while (count <= 1) {
		i = epoll_wait(epollfd, &ev, 1, -1);
		if (i == -1) {
			perror("GPIO: Poll Wait fail");
			count = 5;
		}
		else {
			count++;
		}
	}

	close(fd);
	if (count == 5)
		return -1;
	return 0;
}


int GPIO::waitEdge(CallbackType callback, void* arg) {
	this->threadRunning = true;
	this->callbackFunction = callback;
	this->callbackArgument = arg;

	if (pthread_create(&this->thread, 
						NULL,
						threadedPoll,
						static_cast<void*>(this))) {

		perror("GPIO: Failed to create the poll thread");
    	this->threadRunning = false;
    	return -1;
	}

	return 0;
}


void *threadedPoll(void *value) {
	GPIO *gpio = static_cast<GPIO*>(value);
	while (gpio->threadRunning) {
		if (gpio->waitEdge() == 0) {

			GPIO_EDGE currentEdge = gpio->getEdgeType();

			if (gpio->interruptEdge == NONE) {
				continue;
			}
			if (gpio->interruptEdge == currentEdge || gpio->interruptEdge == BOTH) {
				gpio->callbackFunction(gpio->callbackArgument);
			}
			if (currentEdge == RISING) {
				gpio->setEdgeType(FALLING);
			}
			else if (currentEdge == FALLING) {
				gpio->setEdgeType(RISING);
			}
		}
		usleep(gpio->debounceTime * 1000);
	}

	return 0;
}


void GPIO::stopWaitingEdge() {
	this->threadRunning = false;
}
} /* namespace BBB */