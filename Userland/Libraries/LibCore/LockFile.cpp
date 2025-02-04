/*
 * Copyright (c) 2021, Peter Elliott <pelliott@ualberta.ca>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibCore/File.h>
#include <LibCore/LockFile.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

namespace Core {

LockFile::LockFile(char const* filename, Type type)
    : m_filename(filename)
{
    if (!Core::File::ensure_parent_directories(m_filename))
        return;

    m_fd = open(filename, O_RDONLY | O_CREAT, 0666);
    if (m_fd == -1) {
        m_errno = errno;
        return;
    }

    if (flock(m_fd, LOCK_NB | ((type == Type::Exclusive) ? LOCK_EX : LOCK_SH)) == -1) {
        m_errno = errno;
        close(m_fd);
        m_fd = -1;
    }
}

LockFile::~LockFile()
{
    release();
}

bool LockFile::is_held() const
{
    return m_fd != -1;
}

void LockFile::release()
{
    if (m_fd == -1)
        return;

    unlink(m_filename);
    flock(m_fd, LOCK_NB | LOCK_UN);
    close(m_fd);

    m_fd = -1;
}

}
