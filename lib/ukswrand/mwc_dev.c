/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Vlad-Andrei Badoiu <vlad_andrei.badoiu@stud.acs.upb.ro>
 *
 * Copyright (c) 2019, University Politehnica of Bucharest. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#include <stdlib.h>
#include <string.h>
#include <uk/ctors.h>
#include <uk/print.h>
#include <uk/swrand.h>
#include <uk/assert.h>
#include <uk/config.h>
#include <uk/essentials.h>
#include <vfscore/uio.h>
#include <devfs/device.h>

#define DEV_RANDOM_NAME "random"
#define DEV_URANDOM_NAME "urandom"

int dev_random_read(struct device *dev __unused, struct uio *uio,
			int flags __unused)
{
	size_t count, step, chunk_size, i;
	char *buf;
	__u32 rd;

	buf = uio->uio_iov->iov_base;
	count = uio->uio_iov->iov_len;

	step = sizeof(__u32);
	chunk_size = count % step;

	for (i = 0; i < count - chunk_size; i += step)
		*(buf + i) = uk_swrand_randr();

	/* fill the remaining bytes of the buffer */
	if (chunk_size > 0) {
		rd = uk_swrand_randr();
		memcpy(buf + i, &rd, chunk_size);
	}

	uio->uio_resid = 0;
	return 0;
}

int dev_random_open(struct device *device __unused, int mode __unused)
{
	return 0;
}


int dev_random_close(struct device *device __unused)
{
	return 0;
}

static struct devops random_devops = {
	.read = dev_random_read,
	.open = dev_random_open,
	.close = dev_random_close,
};

static struct driver drv_random = {
	.devops = &random_devops,
	.devsz = 0,
	.name = DEV_RANDOM_NAME
};

static struct driver drv_urandom = {
	.devops = &random_devops,
	.devsz = 0,
	.name = DEV_URANDOM_NAME
};

__constructor_prio(102) static void _uk_dev_swrand_ctor(void)
{
	struct device *dev;

	uk_pr_info("Add /dev/random and /dev/urandom\n");

	/* register /dev/urandom */
	dev = device_create(&drv_urandom, DEV_URANDOM_NAME, D_CHR);
	if (dev == NULL)
		uk_pr_info("Failed to register /dev/urandom\n");

	/* register /dev/random */
	dev = device_create(&drv_random, DEV_RANDOM_NAME, D_CHR);
	if (dev == NULL)
		uk_pr_info("Failed to register /dev/random\n");
}
