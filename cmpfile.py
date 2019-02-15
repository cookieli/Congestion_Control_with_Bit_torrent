f1 = open("example/B.tar", 'rb');
f2 = open("c.tar", 'rb')
line1 = f1.read()
line2 = f2.read()

for i in range(len(line1)):
    if line1[i] != line2[i]:
        print(i)
f1.close()
f2.close()
