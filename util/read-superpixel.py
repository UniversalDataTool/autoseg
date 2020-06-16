from PIL import Image
import numpy as np
import pathlib
from matplotlib import pyplot as plt


img = Image.open(pathlib.Path(__file__).parent.absolute() / "orange.png")

d = np.fromfile(
    pathlib.Path(__file__).parent.absolute() / "../superpixel.bin", dtype=np.uint8
).reshape((249, 320, 4))

# plt.imshow(img)
# plt.imshow(d, alpha=0.5)

plt.imshow(d)
plt.show()
