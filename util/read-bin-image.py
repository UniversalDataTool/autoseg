from PIL import Image
import numpy as np
import pathlib
import sys
from matplotlib import pyplot as plt


img = Image.open(pathlib.Path(__file__).parent.absolute() / "orange.png")

d = np.fromfile(
    sys.argv[1],
    # pathlib.Path(__file__).parent.absolute() / s"../tests/modulejs/mask.bin",
    dtype=np.uint8,
).reshape((249, 320, 4))

plt.imshow(img)
plt.imshow(d, alpha=0.5)

# plt.imshow(d)
plt.show()
