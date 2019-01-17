import sys, getopt
import string
import subprocess

subprocess.call(['./P2P_env.sh'])
alphabets=list(string.ascii_lowercase)
f = open('all.masterchunks')
num_lines = sum(1 for line in f)
argv = sys.argv[1:]
chunk_nums=0
nodes_num=0
try:
    opts, args = getopt.getopt(argv,"hn:m:")
except getopt.GetoptError:
    print("script.py -n number-m number number1 number2 number3...")
    sys.exit(2)
for opt, arg in opts:
    #print(opt, arg)
    if opt == '-h':
        print("script.py -n number number1 number2 number3...")
        sys.exit()
    elif opt in ("-n"):
        chunk_nums=int(arg)
    elif opt in ("-m"):
        nodes_num=int(arg)
    else:
        print("script.py -n number -m number number1 number2 number3...")
if len(args) != chunk_nums:
    print("number's num must equal to chunk_nums")
nums=list(map(int, args))
if sum(nums) != num_lines - 2:
    print("can't match num_lines")


extension='.haschunks'
f.seek(0, 0)

for i,line in enumerate(f):
    if i == 1:
        break;

for i in range(chunk_nums):
    filename=alphabets[i]+extension
    fn=open(filename, 'w+')
    count = 0
    while count < nums[i]:
        s = f.readline()
        fn.write(s)
        count += 1
    fn.close()

f.close()

node_f = open('nodes.map', 'w+')
localhost='127.0.0.1'
for i in range(1, nodes_num+1):
    s = str(i) + ' '+ localhost + ' '+ str(1111*i)+'\n'
    node_f.write(s)
node_f.close()
#print(num_lines)

