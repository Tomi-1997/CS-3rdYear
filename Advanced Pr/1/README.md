# -SHELL IMPLEMENTATION IN C-

📎 Execute commands (ls, whereis, man)<br>
📎 Set and read local variables<br>
📎 Redirect io<br>
📎 Navigate between previous commands, and append to them (Arrow up and enter, Arrow down and enter)<br>
📎 Pipe commands<br>
📎 Write an if block<br>

![D](https://github.com/Tomi-1997/CS-3rdYear/blob/main/Advanced%20Pr/1/demo.gif) <br>

# - User Guide -
📎 Type regular commands with flags such as ls, and chain them with | between them. <br>
```
ls -l
ls | grep myfile
```
📎 Change prompt <br>
```
prompt = good morning:
```
📎 Redirect io by typing after the last command <, >>, >, or 2> <br>
```
date > todayis
```
📎 Perform certain commands with an IF block, the format is as follows <br>
- if (cmd)
- then
- (cmds or empty)
- else
- (cmds or empty)
- fi
```
if ls | grep badfile
then
rm badfile
echo file removed
else
echo no such file
fi
```
```
if date | grep Sat
then
echo Shabat Shalom
else
fi
```
📎 Initialize local variables, single or multiple, $name = val, or by read v1 v2 and after enter typing the values <br>
```
$cmd = ls
$cmd (performs ls and usual)
read cmd flags
ls -l (performs ls with the flag -l)
```
