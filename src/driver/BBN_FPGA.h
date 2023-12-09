/*
 * SPDX-FileCopyrightText: Copyright 2014 Raytheon BBN Technologies. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * Author: Colm Ryan <cryan@bbn.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>


#define VENDOR_ID 0x1172
#define DEVICE_ID 0xbb4e

#define DRIVER_NAME "pcie_fpga"
#define BOARD_NAME "pciecomm1"
#define NUM_BARS 3  // BARS defined in upstream pcie driver


// fileio.c functions for device
int fpga_open(struct inode *inode, struct file *file);
int fpga_close(struct inode *inode, struct file *file);
ssize_t fpga_read(struct file *file, char __user *buf, size_t count, loff_t *pos);
ssize_t fpga_write(struct file *file, const char __user *buf, size_t count, loff_t *pos);


/* Maximum size of driver buffer (allocated with kalloc()).
 * Needed to copy data from user to kernel space */
static const size_t BUFFER_SIZE = PAGE_SIZE;

//Keep track of bits and bobs that we need for the driver
struct DevInfo_t {
  /* the kernel pci device data structure */
  struct pci_dev *pciDev;

  /* upstream root node */
  struct pci_dev *upstream;

  /* kernel's virtual addr. for the mapped BARs */
  void * __iomem bar[NUM_BARS];

  /* length of each memory region. Used for error checking. */
  size_t barLengths[NUM_BARS];

  /* temporary buffer. If allocated, will be BUFFER_SIZE. */
  char *buffer;

  /* Mutex for this device. */
  struct semaphore sem;

  /* PID of process that called open() */
  int userPID;

  /* character device */
  dev_t cdevNum;
  struct cdev cdev;
  struct class *myClass;
  struct device *device;

};
