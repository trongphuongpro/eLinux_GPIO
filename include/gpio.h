#ifndef __GPIO__
#define __GPIO__

#include <string>
#include "pinmap.h"

#define GPIO_PATH	"/sys/class/gpio/"

namespace BBB {

typedef int (*CallbackType)(void*);


class GPIO {
public:
	enum DIRECTION {INPUT, OUTPUT};
	enum VALUE {LOW=0, HIGH=1};
	enum EDGE {NONE, RISING, FALLING, BOTH};

	GPIO(int number, DIRECTION mode);
	~GPIO();

	virtual int getPin();

	virtual int setPinMode(DIRECTION mode);
	virtual DIRECTION getPinMode();

	virtual int write(VALUE value);
	virtual VALUE read();

	virtual int setActiveMode(VALUE mode);

	virtual void setDebounceTime(int time);


	virtual int toggle();
	virtual int toggle(int time);
	virtual int toggle(int numberOfTimes, int frequency);
	virtual void changeToggleFrequency(int time);
	virtual void stopToggle();

	virtual void setInterruptMode(EDGE type);
	virtual int waitEdge();
	virtual int onInterrupt(CallbackType callback, void* arg=NULL);
	virtual void stopInterrupt();


private:
	int pin, debounceTime;
	EDGE interruptEdge;
	std::string path;

	pthread_t thread;

	CallbackType callbackFunction;
	void* callbackArgument;

	bool threadRunning;
	int toggleFrequency;
	int toggleNumber;

	int writeFile(std::string path, std::string filename, std::string value);
	int writeFile(std::string path, std::string filename, int value);
	std::string readFile(std::string path, std::string filename);

	int setEdgeType(EDGE type);
	EDGE getEdgeType();

	friend void *threadedPoll(void *value);
	friend void *threadedToggle(void *value);
};

void *threadedPoll(void *value);
void *threadedToggle(void *value);

} /* namespace BBB */

#endif /* __GPIO__ */