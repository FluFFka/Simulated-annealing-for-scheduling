from random import uniform
from random import randint
# Number of processors
S = int(input())
# Limit time:
T = int(input())
# Limits of task duration (separator - comma)
task_lims = eval(input())
# Limits of proportions of processors speed (separator - comma)
prop_lims = eval(input())

speeds = [uniform(prop_lims[0], prop_lims[1]) for i in range(S)]
schedule = [[0] for i in range(S)]
for i in range(S):
    while schedule[i][-1] < T:
        schedule[i].append(schedule[i][-1] + randint(int(task_lims[0] * speeds[i]), int(task_lims[1] * speeds[i])))
        if schedule[i][-2] == schedule[i][-1]:
            schedule[i][-1] += 1
        if schedule[i][-1] >= T:
            schedule[i][-1] = T
            schedule[i] = schedule[i][1:] # delete first zero

N = 0
for sc in schedule:
    N += len(sc)
print(N, S)
durations = [[0 for i in range(N)] for j in range(S)]
curr_proc = 0
i = 0
for sc in schedule:
    for task in range(len(sc)):
        for j in range(S):
            if task == 0:
                task_time = sc[task]
            else:
                task_time = sc[task] - sc[task - 1]
            if curr_proc == j:
                durations[j][i] = task_time
            else:
                durations[j][i] = int(speeds[j] * task_time / speeds[curr_proc]) + 1
                # durations[j][i] = durations[j][i] if durations[j][i] > 0 else 1
        i += 1
    curr_proc += 1
for i in range(S):
    for j in range(N):
        print(durations[i][j], end=' ')
    print()

# Limits of processor transmitions (separator - comma)
trans_lims = eval(input())
# Edge probability
prob = eval(input())

graph = [[0 for i in range(N)] for j in range(N)]
trans_times = [[0 for i in range(S)] for j in range(S)]
for proc1 in range(S):
    for proc2 in range(S):
        if proc1 == proc2:
            trans_times[proc1][proc2] = 0
        else:
            trans_times[proc1][proc2] = randint(*trans_lims)
for proc1 in range(S):
    for proc2 in range(S): # from proc1 -> proc2
        for task1 in range(len(schedule[proc1])):
            for task2 in range(len(schedule[proc2])):
                task2_start = 0 if task2 == 0 else schedule[proc2][task2 - 1]
                task1_finish = schedule[proc1][task1]
                ss1 = sum([len(schedule[i]) for i in range(proc1)])
                ss2 = sum([len(schedule[i]) for i in range(proc2)])
                task1_num = ss1 + task1
                task2_num = ss2 + task2
                if task1_num < task2_num:
                    if 0 <= task2_start - task1_finish <= trans_times[proc1][proc2]:
                        if uniform(0, 1) <= prob:
                            graph[task1_num][task2_num] = 1
                        else:
                            graph[task1_num][task2_num] = 0
for tr in trans_times:
    for num in tr:
        print(num, end=' ')
    print()
for dep in graph:
    for num in dep:
        print(num, end=' ')
    print()
