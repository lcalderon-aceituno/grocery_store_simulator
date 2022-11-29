#include <iostream>
#include <vector>
#include <thread>
#include <tuple>
#include <algorithm>    // std::sort
#include <mutex>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include "../headers/mingw.thread.h"

using namespace std;


// For manipulating vectors 
template <typename T>
void remove(std::vector<T>& v, size_t index) {
	v.erase(v.begin() + index);
}


// Custom type to identify event type 
enum type {
	arriveAtStore,
	arriveAtCheckout,
	depart
};

// Stream output operator for custom type 
ostream& operator<< (ostream& out, type t)
{
	switch (t)
	{
	case arriveAtStore: out << "ArriveAtStore"; break;
	case arriveAtCheckout: out << "ArriveAtCheckout"; break;
	case depart: out << "Depart"; break;
	}
	return out;
}

// Customer class holds relevant information about individual customers 
class customer {
public:
	vector<tuple<string, float>> shoppingCart; // Each customer has a personal shopping cart that carries items and their price as a tuple 
	int id = 0; // Customer ID
	int cashierLine; // Tracks which cashier this customer ends up at 

	// For customers that wait in line, these values will 
	// be updated to reflect when they arrived in line and
	// when they finally left the store
	float timeArrivedInLine = 0; 
	float timeSeenByCashier = 0;

	// Default constructor
	customer() {} 

	// Overloaded constructor assigns ID
	customer(int counter) : id(counter) {} 

	// Adds items to customer's shopping cart 
	void addToCart(tuple<string, float> item) {
		shoppingCart.push_back(item);
	}

};

class _groceryEvent {
public: 
	int eventNumber; // Tracks how which event this is in the history of the simulation
	float eventTime; // Tracks at what time this event is supposed to happen 
	type eventType; // Event type 
	customer shopper; // Customer that is excecuting this event 
	
	// Default constructor 
	_groceryEvent() {} 

	// Overloaded constructor 
	_groceryEvent(int num, float time, type type, customer person) : eventNumber(num), eventTime(time), eventType(type), shopper(person) {} 


	// Getters 
	int number() { return eventNumber; }
	float time() { return eventTime; }
	type type() { return eventType; }
};

// Generic simulation class which tracks time 
class simulation {
public:
	float currentTime; 
	float totalSimulationTime; 
	simulation(float totalSimulationTime) : currentTime(0) {
		this->totalSimulationTime = totalSimulationTime; // Set total simulation time
	}

	// Getters 
	float getCurrentTime() { return currentTime;  }
	float getTotalTime() { return totalSimulationTime; }
	
	// Setters
	void setCurrentTime(float newTime) { currentTime = newTime;  }
};


// Specific type of simulation: grocery store
class groceryStore : public simulation {
public:
	int timePerItem; // Seconds to checkout per item 
	int numberOfEvents; // Number of events that have happened in this grocery store 
	int customerCounter; // Number of customers that have come to the store 
	float groceryStoreRevenue; // Running total of revenue 
	float totalWaitingTime; // Running total of time spent by each customer waiting in line 
	vector<tuple<string, float>> items; // Items avaliable for purchace, in tuples of <string, int> for <product_name, price>
	vector<_groceryEvent> FEL; // Future event list holding list of grocery-store specific events 

	// Getters 
	int getCustomerCount() { return customerCounter; }
	int getEventNumer() { return numberOfEvents; }

	// Setters 
	void setCustomerCount(int count) { customerCounter = count; }

	
	// Vector for each cashier, when a customer gets in line they will be pushed back into the appropriate vector 
	vector<customer> cashier1;
	vector<customer> cashier2;
	vector<customer> cashier3;

	// Returns 1 if the 1st cashier is most free, 2 if second...
	int mostFreeCashier() {
		size_t smallestQueue = 1001;
		int lineNum;

		if (cashier1.size() < smallestQueue) {
			smallestQueue = cashier1.size();
			lineNum = 1;
		}
		if (cashier2.size() < smallestQueue) {
			smallestQueue = cashier2.size();
			lineNum = 2; 
		}
		if (cashier3.size() < smallestQueue) {
			smallestQueue = cashier3.size();
			lineNum = 3; 
		}

		return lineNum;
	}

	// Returns the cashier number which has no queue
	// Returns 0 if all cashiers have a queue
	int aCashierIsFree() {
		if (cashier1.size() == 0) {
			return 1; 
		}
		else if (cashier2.size() == 0) {
			return 2; 
		}
		else if (cashier3.size() == 0) {
			return 3; 
		}
		else {
			return 0; 
		}
	}

	// Constructor
	// // Sets up the items vector, and all initial grocery store parameters 
	groceryStore(float totalSimulationTime) : simulation(totalSimulationTime){
		items.push_back(make_tuple("apples", 50));
		items.push_back(make_tuple("banana", 2));
		items.push_back(make_tuple("peaches", 69));
		items.push_back(make_tuple("avacado", 80));
		items.push_back(make_tuple("celery", 5));
		items.push_back(make_tuple("box", 7));
		items.push_back(make_tuple("shoe", 14));
		items.push_back(make_tuple("thing", 90));
		items.push_back(make_tuple("thing1", 15));
		items.push_back(make_tuple("thing2", 80));

		// All of these are initially 0
		numberOfEvents = 0;
		customerCounter = 0; 
		groceryStoreRevenue = 0; 
		totalWaitingTime = 0; 
		// We decide it will take 10 seconds to checkout each item 
		timePerItem = 10;
	}

	// Event type methods 
	auto arrivalEvent(_groceryEvent e) {
		cout << "Customer " << e.shopper.id << " has arrived!! The time is " << currentTime << " seconds " << endl; // Update user of how the simulation is going 
		int randomNumberOfGroceries = rand() % 10 + 1; // Randomly decide how many groceries this customer will buy
		int itemIndex; // Index of the grocery customer will pick off the shelves 
		float totalCheckoutTime;
		float revenueFromThisSale = 0;

		for (int i = 0; i < randomNumberOfGroceries; i++) {
			itemIndex = rand() % 10; // Decide on a random object
			e.shopper.addToCart(items[itemIndex]);
			//shoppingCart.push_back(items[itemIndex]); // Add that object to the shopping cart 
			revenueFromThisSale += get<1>(items[itemIndex]); // Add price of this object to the running total 
		}

		this_thread::sleep_for(std::chrono::milliseconds(rand() % 500)); // wait for a bit

		groceryStoreRevenue += revenueFromThisSale; // Add this sale's revenue to the total store revenue 
		totalCheckoutTime = float(timePerItem * randomNumberOfGroceries); // Assume it takes 5 seconds to checkout each item 

		//// Push new checkout event to FEL
		//// This event has updated event number, current time + total amount of time to checkout, arrival type, and customer ID
		 
		numberOfEvents++; // increment number of events 
		_groceryEvent gotocheckout(numberOfEvents, currentTime + totalCheckoutTime, arriveAtCheckout, e.shopper);
		FEL.push_back(gotocheckout);


		// Pushback the arrival of another customer
		// 
		// We want totally to have 1000 customers 
		// Lets say 100 of those come within the first three hours (10:00am - 1pm)
		// // so there are ~108 seconds between each customer in the first 0 - 10,800 seconds 
		// 200 come in the two hours after that (1pm - 3pm) 
		// // there are 36 seconds between each customer in this time window of 10,800 - 18,000 seconds 
		// 500 come  in the afternoon peak hours (3:00pm - 6:00pm) 
		// // 21.6 seconds between each customer in this time window of 18,000 - 28,800 seconds 
		// 200 come later at night (6:00pm - 8:00pm) 
		// // 36 seconds between each customer from 28,000 - 36,000 seconds
		float timeInterval = 0;
		if (currentTime < 10800) {
			timeInterval = float(108.5);
		}
		else if ((currentTime > 10800) && (currentTime < 18000)) {
			timeInterval = float(36);
		}
		else if ((currentTime > 18000) && (currentTime < 28800)) {
			timeInterval = float(21.6);
		}
		else {
			timeInterval = float(36);
		}

		numberOfEvents++; // Increment number of events in preperation of new event 
		customerCounter++; // Increment number of customers in preperation of new customer arriving
		customer next(customerCounter);
		_groceryEvent arrival(numberOfEvents, currentTime + timeInterval, arriveAtStore, next);
		FEL.push_back(arrival); // Push this event to FEL 
	}

	auto arrivalAtCheckoutEvent(_groceryEvent e) {
		cout << "Customer " << e.shopper.id << " has arrived at the checkout " << endl;
		// if the cashier is free checkout 
		int freeOne = aCashierIsFree();
		int checkoutTime = 0;
		if (freeOne) { // If a cashier is free
			// Calculate the amount of time it will take to checkout 
			checkoutTime = e.shopper.shoppingCart.size() * timePerItem; // Assume number of seconds for each item in shopping cart
			numberOfEvents++; // increment number of events 
			e.shopper.cashierLine = freeOne; 
			switch (freeOne) {
				case 1: 
					cashier1.push_back(e.shopper);
					break; 
				case 2: 
					cashier2.push_back(e.shopper);
					break;
				case 3: 
					cashier3.push_back(e.shopper);
					break;
			}
			_groceryEvent checkout(numberOfEvents, currentTime + checkoutTime, depart, e.shopper); // push back a departure event 
			FEL.push_back(checkout);
		} else { // If no cashier is free, push them into the checkout line of a cashier 
			// For customers that are waiting in line, we should calculate the total amount of time they spend in line 
			e.shopper.timeArrivedInLine = currentTime; // Log the time that the shopper is placed in line 
			int mostFree = mostFreeCashier();
			switch (mostFree) {
				case 1:
					e.shopper.cashierLine = 1;
					cashier1.push_back(e.shopper);
					break; 
				case 2: 
					e.shopper.cashierLine = 2;
					cashier2.push_back(e.shopper);
					break; 
				case 3: 
					e.shopper.cashierLine = 3;
					cashier3.push_back(e.shopper);
					break; 

			}
		}
	}

	auto departureEvent(_groceryEvent e) {
		cout << "Customer " << e.shopper.id << " is leaving the store from cashier line " << e.shopper.cashierLine << endl;

		// A customer's departure event must consider the next customer waiting in line, if there is one
		
		// If there is a customer waiting behind the current customer, we need to know:
		int checkoutTime = 0; // Time it will take to check out 
		customer nextInLine; // Other relevant information about the customer

		switch (e.shopper.cashierLine) { // Grab the checkout time of the customer first in line in this cashier's line 
			case 1: 
				cashier1.erase(cashier1.begin()); // remove checkedout customer from the waiting line 
				if (cashier1.size() != 0) { // If there is a customers waiting behind  
					nextInLine = cashier1[0]; // Grab next customer in line 
				}
				break;
			case 2:
				cashier2.erase(cashier2.begin()); // remove checkedout customer from the waiting line 
				if (cashier2.size() != 0) { // If there is a customers waiting behind  
					nextInLine = cashier2.at(0); // Grab next customer in line
				}
				break; 
			case 3:  
				cashier3.erase(cashier3.begin()); // remove checkedout customer from the waiting line
				if (cashier3.size() != 0) { // If there is a customers waiting behind  
					nextInLine = cashier3.at(0); // Grab next customer in line
				}
				break; 		
		}

		if (nextInLine.id != 0) { // If there was a customer next in line, push their departure event to FEL and record how much time they were waiting for 
			nextInLine.timeSeenByCashier = currentTime; // Record the time they were seen by cashier 
			totalWaitingTime += (nextInLine.timeSeenByCashier - nextInLine.timeArrivedInLine); // Add the amount of time waited by this particular customer to the running total 
			numberOfEvents++; // Increment number of events 
			checkoutTime = nextInLine.shoppingCart.size() * timePerItem; // Calculate the amount of time it'll take to checkout 
			_groceryEvent leave(numberOfEvents, currentTime + checkoutTime, depart, nextInLine); // Schedule checkout time of the customer first in line 
			FEL.push_back(leave);
		}
	}

	// Finds imminent event based on time 
	tuple<_groceryEvent, int> findImminentEvent() {
		// Find event in FEL with time stamp closest to the current simulation time
		int index; 
		float difference = 10000000000; // create huge initial difference
		for (size_t i = 0; i < FEL.size(); i++) {
			if (abs(FEL[i].eventTime - currentTime) < difference) {
				difference = abs(FEL[i].eventTime - currentTime); // update new smallest difference 
				index = i;  // update the index tracker to the index of the most imminent event 
			}
		}
		return make_tuple(FEL[index], index);
	}

	// Prints current grocery store stats including each cashier's line, running revenue, and average waiting time 
	void checkStatus() {
		cout << "------------------------------------------------------" << endl;
		cout << "Cashier 1's line: ";
		for (size_t i = 0; i < cashier1.size(); i++) {
			cout << cashier1[i].id << " ";
		}
		cout << endl;
		cout << "Cashier 2's line: ";
		for (size_t i = 0; i < cashier2.size(); i++) {
			cout << cashier2[i].id << " ";
		}
		cout << endl;
		cout << "Cashier 3's line: ";
		for (size_t i = 0; i < cashier3.size(); i++) {
			cout << cashier3[i].id << " ";
		}
		
		cout << endl; 
		cout << endl; 
		cout << " Total store revenue: $" << groceryStoreRevenue << endl; 
		cout << " Average waiting time is " << totalWaitingTime / customerCounter << " seconds " << endl; 
		cout << "------------------------------------------------------" << endl;
	}
};


int main() {
	float totalSimulationTime = 36000; // Ten hours cut up into seconds (10*60*60)
	groceryStore theStore(totalSimulationTime);

	_groceryEvent imminentEvent;
	int index;
	float currentTime; 

	_groceryEvent firstArrival(theStore.getEventNumer(), 0, arriveAtStore, theStore.getCustomerCount());
	theStore.FEL.push_back(firstArrival);

	while (theStore.getCurrentTime() < totalSimulationTime) {
		tie(imminentEvent, index) = theStore.findImminentEvent();
		theStore.setCurrentTime(imminentEvent.time()); // Update store time 
		remove(theStore.FEL, index);
		
		switch (imminentEvent.type()) {
			case arriveAtStore:
				theStore.arrivalEvent(imminentEvent);
				break;
			case arriveAtCheckout:
				theStore.arrivalAtCheckoutEvent(imminentEvent);
				break; 
			case depart:
				theStore.departureEvent(imminentEvent);
				break; 
		}

		theStore.checkStatus();
	}

	cout << "SIMULATION COMPLETE" << endl;
	cout << "Final stats: " << endl; 
	cout << " - Total grocery store revenue was $" << theStore.groceryStoreRevenue << endl; 
	cout << " - Average waiting time per customer was " << float(theStore.totalWaitingTime / theStore.customerCounter) << " seconds "<< endl;
}