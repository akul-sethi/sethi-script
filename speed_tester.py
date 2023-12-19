import time
import os

oldStart = time.time()
os.system("./sethi_old file_test.sethi")
oldDuration = time.time() - oldStart

startSethi = time.time()
os.system("./sethi file_test.sethi")
sethiDuration = time.time() - startSethi


print("SethiScript time to complete: ", sethiDuration)
print("Old time to complete", oldDuration)
