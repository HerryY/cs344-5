'''
Geoffrey Pard
CS 344 - 400
Program 5

This Program creates three new files that contain 
10 lower case random letters.  When the program 
executes the contents of each file will be printed.
It also does some minor math as stipulated in the project
details.
'''
import sys
import random
import string


#Delcare Variables
content_array = []
lower_letters = ""

print("\n")
print("Three Files -- 10 Random Lower Letters Each:\n")


# This loop creates 3 strings
# The inner loop fills eachs string with random lowercase letters
# Using built-in ascii lowercase Python function
for i in range(0,3):
   for j in range(0,10):
      lower_letters = lower_letters + str(random.choice(string.ascii_lowercase))

   content_array.append(lower_letters)

   # Create files and fill with elements in array
   f = "file" + str(i+1) + ".txt"
   f2 = open(f, 'w')
   f2.write(content_array[i] + "\n")
   f2.close()
 
   # Output File contents to console
   print("File " + str(i+1) + " contents: " + content_array[i])

   # Reset lower_letters for next iteration
   lower_letters = ""
      
print("\n")
'''
Find the product of two random ints
in the range of (1, 42)
'''

#Declare Variables
num1 = 0
num2 = 0
product = 0

num1 = random.randint(1, 42)
num2 = random.randint(1, 42)

product = num1 * num2

print("---------------------------")
print("Random Numbers Multiplied")
print("Random Number 1: " + str(num1))
print("Random Number 2: " + str(num2))
print(str(num1) + " X " + str(num2) + " = " + str(product))  
print("\n")
      
