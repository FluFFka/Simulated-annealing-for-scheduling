from random import randint
from random import uniform
# Number of tasks
N = int(input())
# Number of processors
S = int(input())
print(N, S)
# Limits of task duration (separator - comma)
task_lims = eval(input())
for i in range(S):
    for j in range(N):
        print(randint(*task_lims), end=' ')
    print()
# Limits of processor transmitions (separator - comma)
trans_lims = eval(input())
for i in range(S):
    for j in range(S):
        print(randint(*trans_lims), end=' ')
    print()
# Edge probability
prob = eval(input())
for i in range(N):
    for x in range(i + 1):
        print(0, end=' ')
    for j in range(i+1, N):
        if i == j:
            print(0, end=' ')
        else:
            if uniform(0, 1) <= prob:
                print(1, end=' ')
            else:
                print(0, end=' ')
    print()