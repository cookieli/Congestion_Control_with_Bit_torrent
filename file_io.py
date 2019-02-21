file1 = 'example/B.tar'
file2 = 'example/C.tar'
output_file = 'f.tar'
f1 = open(file1, 'rb')
f2 = open(file2, 'rb')
h1 = f1.read()
h2 = f2.read()
f3 = open(output_file, 'wb')
f3.write(h1)
f3.write(h2)
f1.close()
f2.close()
f3.close()
