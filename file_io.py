import sys
files=list()
argc = len(sys.argv)
output_file = 'output.tar'
o = open(output_file, 'wb')
for n in range(1, argc):
    files.append(open(sys.argv[n], 'rb'))

for f in files:
    #print("write files")
    h = f.read()
    o.write(h)
    #sys.exit(0)
    #print("after write")
    f.close()
o.close()