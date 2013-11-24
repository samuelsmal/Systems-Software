#README.md

The program terminates only if the user enters any key and hits ENTER.

##Thoughts about starvation:
I made two critical sections: The obvious one with the food stand. The second on with a queue.
My visitors are English and form a orderly queue and wait till each time has come. When they received their food they'll eat and wait.

I have stored my old version under "food_festival_sim_beta.cpp", which uses a different method, but simulates the behaviour of the problem better - if you ask me.
The visitors are rude and want to get to the food by all means. And have the possibility to block other visitors (read their threads).
It uses an additional check before requesting the lock. But this only reduces the possibility of starvation.

##About the structure of the program:
I use four (plus one master thread) different kind of threads (or threads which execute four different functions):

* Input
	void* handleInput(void *);
* Handling the attendant
	void* handleAttendant(void* arg);
* Handling the visitors individual behaviour
	void* handleVisitor(void* arg);
* Handling the visitors' queue
	void* handleFoodDistribution(void* arg);
	
The only complicated part is the interplay between handleVisitor and handleFoodDistribution:
Each visitor enqueues himself, is dequeued by the queue and then start to eat. Then he enqueues himself again.

Each visitor can be in three states:
Waiting in line, eating, or neither of those. The correctness of this behaviour is guaranteed by the interaction between handleVisitor, and handleFoodDistribution.

Anyway the structur of the program is over complicated. Lots of pointers and heap allocation... I tried to make it clear through lots of comments, but I'm not satisfied with the end result.
I hoped to use at least unique_ptr and shared_ptr but gcc4.8 still complaines and clang won't find stdlib... So yeah, raw pointers...

##Memory:
The attendant is deleted on the end of the runAttendant method, since he has fullfilled his usefullness.
The food_stand is a global variable not the free store, needs no extra delete call.
The visitors are deleted on the end of the output section.

Ownership (Beta version):
The attendant owns the food stand is responsible for his memory management.

##About the waiting time after giving the abort signal (the "any keystroke"):
I first planned on calling "int pthread_cancel(pthread_t tid)" on every thread after receiving the abort signal, then I thought that not joining them with the master thread would do the same (since they do not continue on eating), but I the end I keept it the way it is now, because otherwise I would have a possible memory leak.
I am aware that there are ways to handle the cancel request, but I didn't had the time to finish my readings... Sorry about that.
