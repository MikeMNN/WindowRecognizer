# WindowRecognizer
Test task which asked to make a Windows application which could claim window infoermation under the cursor

It starts to analyse mouse events after button is pressed and never ends, but shows and could save window class name, title and app ptah for windoqw. That was the task aim

It uses Windows hook system. Possibly could be swaped with SetCapture-ReleaseCapture mechanism, but in current task it is not a best idea from my point of view due to the fact
that we never end to looking for MouseEvents and SetCapture bloccks everything except this task.
