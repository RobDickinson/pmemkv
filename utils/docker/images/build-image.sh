#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2016-2020, Intel Corporation

#
# build-image.sh <OS-VER> - prepares a Docker image with <OS>-based
#                           environment for testing pmemkv, according
#                           to the Dockerfile.<OS-VER> file located
#                           in the same directory.
#
# The script can be run locally.
#

set -e

OS__OS_VER=${1}

function usage {
	echo "Usage:"
	echo "    build-image.sh <OS-VER>"
	echo "where <OS-VER>, for example, can be 'fedora-31', provided " \
		"a Dockerfile named 'Dockerfile.fedora-31' exists in the " \
		"current directory."
}

# Check if the argument is not empty
if [[ -z "${OS__OS_VER}" ]]; then
	usage
	exit 1
fi

if [[ -z "${CONTAINER_REG}" ]]; then
	echo "CONTAINER_REG environment variable is not set"
	exit 1
fi

# Check if the file Dockerfile.OS-VER exists
if [[ ! -f "Dockerfile.${OS__OS_VER}" ]]; then
	echo "Error: Dockerfile.${OS__OS_VER} does not exist."
	echo
	usage
	exit 1
fi

# Build a Docker image tagged with ${CONTAINER_REG}:1.4-OS-VER
docker build -t ${CONTAINER_REG}:1.4-${OS__OS_VER} \
	--build-arg http_proxy=${http_proxy} \
	--build-arg https_proxy=${https_proxy} \
	-f Dockerfile.${OS__OS_VER} .
