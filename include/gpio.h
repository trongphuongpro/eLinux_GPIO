#ifndef __GPIO__
#define __GPIO__

#include <string>
#include <fstream>
#include "pinmap.h"

#define GPIO_PATH	"/sys/class/gpio/"

namespace BBB {

typedef int (*CallbackType)(void*);

enum GPIO_DIRECTION {INPUT, OUTPUT};
enum GPIO_VALUE {LOW=0, HIGH=1};
enum GPIO_EDGE {NONE, RISING, FALLING, BOTH};


class GPIO {
public:
	GPIO(int number, GPIO_DIRECTION mode);
	~GPIO();

	virtual int getPin();

	virtual int setPinMode(GPIO_DIRECTION mode);
	virtual GPIO_DIRECTION getPinMode();

	virtual int write(GPIO_VALUE value);
	virtual GPIO_VALUE read();

	virtual int setActiveMode(GPIO_VALUE mode);

	virtual void setDebounceTime(int time);

	virtual int openStream();
	virtual int writeStream(GPIO_VALUE value);
	virtual int closeStream();

	virtual int toggle();
	virtual int toggle(int time);
	virtual int toggle(int numberOfTimes, int frequency);
	virtual void changeToggleFrequency(int time);
	virtual void stopToggle();

	virtual void setInterruptMode(GPIO_EDGE type);
	virtual int waitEdge();
	virtual int onInterrupt(CallbackType callback, void* arg=NULL);
	virtual void stopInterrupt();


private:
	int pin, debounceTime;
	GPIO_EDGE interruptEdge;
	std::string path;

	std::ofstream stream;
	pthread_t thread;

	CallbackType callbackFunction;
	void* callbackArgument;

	bool threadRunning;
	int toggleFrequency;
	int toggleNumber;

	int writeFile(std::string path, std::string filename, std::string value);
	int writeFile(std::string path, std::string filename, int value);
	std::string readFile(std::string path, std::string filename);

	int setEdgeType(GPIO_EDGE type);
	GPIO_EDGE getEdgeType();

	friend void *threadedPoll(void *value);
	friend void *threadedToggle(void *value);
};

void *threadedPoll(void *value);
void *threadedToggle(void *value);

} /* namespace BBB */

#endif /* __GPIO__ */