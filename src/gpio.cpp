#include "gpio.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <pthread.h>


using namespace std;

namespace BBB {

GPIO::GPIO(int pin, DIRECTION mode) {
	this->pin = pin;
	this->debounceTime = 0;
	this->toggleFrequency = 0;
	this->toggleNumber = 0;
	this->callbackFunction = NULL;
	this->threadRunning = false;
	this->interruptEdge = NONE;

	string filename = "gpio" + to_string(pin);
	cout << "Create object " << filename << endl;

	this->path = GPIO_PATH + filename + "/";

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


int GPIO::writeFile(string path, string filename, string value) {
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


int GPIO::writeFile(string path, string filename, int value){
   return writeFile(path, filename, to_string(value));
}


string GPIO::readFile(string path, string filename) {
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


int GPIO::setPinMode(DIRECTION mode) {
	if (mode == INPUT) {
		return writeFile(this->path, "direction", "in");
	}
	if (mode == OUTPUT) {
		return writeFile(this->path, "direction", "out");
	}
	return -1;
}


GPIO::DIRECTION GPIO::getPinMode() {
	string input = readFile(this->path, "direction");
	if (input == "in")
		return INPUT;
	else
		return OUTPUT;
}


int GPIO::write(VALUE value) {
	if (value == LOW) {
		return writeFile(this->path, "value", "0");
	}
	if (value == HIGH) {
		return writeFile(this->path, "value", "1");
	}
	return -1;
}


GPIO::VALUE GPIO::read() {
	string input = readFile(this->path, "value");
	if (input == "0")
		return LOW;
	else
		return HIGH;
}


void GPIO::setInterruptMode(EDGE type) {
	this->interruptEdge = type;
	this->setEdgeType(type);
}


int GPIO::setEdgeType(EDGE type) {
	if (type == NONE) {
		return writeFile(this->path, "edge", "none");
	}
	if (type == RISING) {
		return writeFile(this->path, "edge", "rising");
	}
	if (type == FALLING) {
		return writeFile(this->path, "edge", "falling");
	}
	if (type == BOTH) {
		return writeFile(this->path, "edge", "both");
	}
	return -1;
}


GPIO::EDGE GPIO::getEdgeType() {
	string input = readFile(this->path, "edge");
	if (input == "rising") 
		return RISING;
	else if (input == "falling") 
		return FALLING;
	else if (input == "both") 
		return BOTH;
	else 
		return NONE;
}


int GPIO::setActiveMode(VALUE mode) {
	if (mode == LOW) {
		return writeFile(this->path, "active_low", "1");
	}
	else if (mode == HIGH) {
		return writeFile(this->path, "active_low", "0");
	}
	return 0;
}


int GPIO::openStream() {
	this->stream.open((this->path + "value").c_str());
	return 0;
}


int GPIO::writeStream(VALUE value) {
	this->stream << value << flush;
	return 0;
}


int GPIO::closeStream() {
	this->stream.close();
	return 0;
}


int GPIO::toggle() {
	if (getPinMode() == INPUT) {
		perror("GPIO: Cannot toggle input pin");
		return -1;
	}
	
	if (read() == LOW)
		write(HIGH);
	else
		write(LOW);

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
	
	this->toggleNumber = numberOfTimes;
	this->toggleFrequency = frequency;
	this->threadRunning = true;

	if (pthread_create(&this->thread, 
						NULL, 
						threadedToggle, 
						this)) {
		
		perror("GPIO: Failed to create the toggle thread");
		this->threadRunning = false;
		return -1;
	}
	
	return 0;
}


void *threadedToggle(void *value) {
	GPIO *gpio = static_cast<GPIO*>(value);
	bool isHigh = (bool)gpio->read();

	while (gpio->threadRunning) {
		if (isHigh) {
			gpio->write(GPIO::LOW);
		}
		else {
			gpio->write(GPIO::HIGH);
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


int GPIO::onInterrupt(CallbackType callback, void* arg) {
	this->threadRunning = true;
	this->callbackFunction = callback;
	this->callbackArgument = arg;

	if (pthread_create(&this->thread, 
						NULL,
						threadedPoll,
						this)) {

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

			GPIO::EDGE currentEdge = gpio->getEdgeType();

			if (gpio->interruptEdge == GPIO::NONE) {
				continue;
			}
			if (gpio->interruptEdge == currentEdge || gpio->interruptEdge == GPIO::BOTH) {
				gpio->callbackFunction(gpio->callbackArgument);
			}
			if (currentEdge == GPIO::RISING) {
				gpio->setEdgeType(GPIO::FALLING);
			}
			else if (currentEdge == GPIO::FALLING) {
				gpio->setEdgeType(GPIO::RISING);
			}
		}
		usleep(gpio->debounceTime * 1000);
	}

	return 0;
}


void GPIO::stopInterrupt() {
	this->threadRunning = false;
}

} /* namespace BBB */