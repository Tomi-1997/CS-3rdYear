# - Threadpool -

Distributing the workload among several threads, lowering runtime by up to 9x! <br>
This is the result of running the program on the example.txt: <br>
![D](https://github.com/Tomi-1997/CS-3rdYear/blob/main/Advanced%20Pr/2/runtime.png) <br>
The program consists of three groups: <br>
⏰ One alarm clock and printer thread<br>
📝 One input writer thread<br>
⚒ Several worker threads<br>

The workflow is as follows :<br>

📝 Input writer inserts to queue work with an id corresponding to it's order. <br>
⚒ Takes work from queue, begins to proccess it.<br>
⚒ Signals that it finished and goes to sleep.<br>
⏰ Sleeps until it was signaled. <br>
⏰ Searches for the next work to be printed.<br>
⏰ Wakes up all workers who are sleeping.<br>
⚒ Wakes up, if there is no work- goes back to sleep.<br>
📝 Once there is no more input- sends dummy work.<br>
⚒ Wakes up to dummy work- terminates.<br>
