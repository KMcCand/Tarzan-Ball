import os
import sys

filenames = []

def check_valid(file_name):
    print(file_name[0:4])
    print(file_name[-3:])
    if file_name[0:5] != "demo/":
        return False
    elif file_name[-3:] != "png":
        return False
    else:
        return True

def image_files(user_input):
    new_input = user_input
    while(not check_valid(new_input)):
        new_input = input("Input valid name, <q> to go back\n")
        if(new_input == "q"):
            return -1
    return new_input

if __name__ == "__main__":
    user_input = input("Input image file, <q> to quit, <n> for next step\n")
    while(user_input != "q" and user_input != "n"):
        user_input = image_files(user_input)
        if(user_input == -1):
            break
        user_input = input("Next image, <q> to quit, <n> for next step\n")

    if(user_input == "q"):
        quit

    