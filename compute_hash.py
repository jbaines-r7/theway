import hashlib


import argparse
import socket
import struct
import sys
import time
import select

top_parser = argparse.ArgumentParser(description='')
top_parser.add_argument('-i', '--input', action="store", dest="infile", required=True, help="The file to validate")
top_parser.add_argument('-o', '--output', action="store", dest="outfile", required=True, help="The file to output")
args = top_parser.parse_args()

infile_handle = open(args.infile ,mode='rb')
all_of_it = infile_handle.read()
infile_handle.close()

header = all_of_it[0:0x70]
data = all_of_it[0x80:]

m = hashlib.md5()
m.update(header)
m.update(data)
print(m.hexdigest())

outfile_handle = open(args.outfile ,mode='wb')
outfile_handle.write(header)
outfile_handle.write(m.digest())
outfile_handle.write(data)
outfile_handle.close()



