from PIL import Image
import numpy as np
import pathlib
import sys
from matplotlib import pyplot as plt
import re

whreg = re.search(r"([0-9]+)x([0-9]+)", sys.argv[1])
width = int(whreg.group(1))
height = int(whreg.group(2))


# img = Image.open(sys.argv[1])

d = np.fromfile(
    sys.argv[1],
    # pathlib.Path(__file__).parent.absolute() / s"../tests/modulejs/mask.bin",
    dtype=np.uint8,
).reshape((height, width, 4))

# plt.imshow(img)
# plt.imshow(d, alpha=0.5)

plt.imshow(d)
plt.show()
