import time
import os

pythonStart = time.time()
os.system('python3 pythonImp.py')
pythonDuration = time.time() - pythonStart

startSethi = time.time()
os.system('./sethi file_test.sethi')
sethiDuration = time.time() - startSethi



print("SethiScript time to complete: ", sethiDuration)
print("Python time to complete", pythonDuration)