/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of qmlgit.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/


#include "gitmodel.h"

#include <qgit2.h>

#include <QDateTime>
#include <QThread>
#include <QDebug>

using namespace LibQGit2;

GitModel::GitModel()
    : m_diffDirty(true)
{

}

void GitModel::setGit(Git *repo)
{
    m_repo = repo;
    connect(m_repo, &Git::repoUrlChanged, this, &GitModel::repoUrlChanged);
    connect(m_repo, &Git::branchesChanged, this, &GitModel::branchesChanged);

    repoUrlChanged();
    emit gitChanged();
}

QHash<int, QByteArray> GitModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(Oid, "oid");
    roles.insert(Author, "author");
    roles.insert(AuthorEmail, "authorEmail");
    roles.insert(Committer, "committer");
    roles.insert(CommitterEmail, "committerEmail");
    roles.insert(Message, "message");
    roles.insert(ShortMessage, "shortMessage");
    roles.insert(ParentCount, "parentCount");
    roles.insert(Time, "time");
    return roles;
}

void GitModel::setBranchData(const QVector<Commit> &data)
{
    beginResetModel();
    m_commits = data;
    endResetModel();
}

int GitModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_commits.count();
}

QVariant GitModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Commit commit = m_commits.at(index.row());

    switch (role) {
    case Message:
        return commit.message();
    case ShortMessage:
        return commit.shortMessage();
    case Author:
        return commit.author().name();
    case AuthorEmail:
        return commit.author().email();
    case Committer:
        return commit.committer().name();
    case CommitterEmail:
        return commit.committer().email();
    case ParentCount:
        return commit.parentCount();
    case Time:
        return commit.dateTime();
    case Oid:
        return QString::fromUtf8(commit.oid().format());
    }
    return QVariant();
}

QString GitModel::currentBranch() const
{
    return m_branch;
}

void GitModel::setCurrentBranch(const QString &branch)
{
    m_branch = branch;
    if (!branch.isEmpty()) {
        Q_ASSERT(m_repo->cache());
        m_repo->cache()->loadBranch(branch);
        emit updateCache();
    }
}

void GitModel::branchLoaded(const QString &branch)
{
    if (branch == m_branch) {
        beginResetModel();
        setBranchData(m_repo->cache()->branchData(branch));
        endResetModel();
    }
}

void GitModel::diffLoaded(const QString &commit)
{
    if (commit == m_commit)
        emit diffChanged();
}

void GitModel::repoUrlChanged()
{
    beginResetModel();
    connect(m_repo->cache(), &GitCache::branchLoaded, this, &GitModel::branchLoaded);
    connect(m_repo->cache(), &GitCache::diffLoaded, this, &GitModel::diffLoaded);
    connect(this, &GitModel::updateCache, m_repo->cache(), &GitCache::doWork);
    m_diffDirty = true;
    m_commits.clear();
    m_commit.clear();
    m_branch.clear();
    m_diff.clear();
    endResetModel();
    emit branchesChanged();
    emit diffChanged();
}

QString GitModel::currentCommit() const
{
    return m_commit;
}

void GitModel::setCurrentCommit(const QString &commit)
{
    m_commit = commit;
    if (!m_commit.isEmpty()) {
        m_repo->cache()->loadDiff(commit);
        emit updateCache();
    }
    emit diffChanged();
}

QString GitModel::diff() const
{
    if (!m_commit.isEmpty())
        return m_repo->cache()->diff(m_commit);
    return QString();
}

void GitModel::moveCommit(int from, int to)
{
//    int oldCommit = from > to ? from : to;
//    int newCommit = from < to ? from : to;

//    OId oldOid;
//    oldOid.fromString(data(index(oldCommit), Oid).toString());

//    OId newOid;
//    newOid.fromString(data(index(newCommit), Oid).toString());

//    Repository repo = m_repo->repository();

//    Commit oldCommit = repo.lookupCommit(oldOid);
//    if (oldCommit.parentCount() != 1) {
//        qWarning() << "Cannot play with merges or the initial commit";
//        return;
//    }

//    Commit newCommit = repo.lookupCommit(newOid);

//    qDebug() << "Old commit: " << repo.lookupCommit(oldOid).message();
//    qDebug() << "New commit: " << repo.lookupCommit(newOid).message();

//    // checkout parent of old commit



//    // cherry-pick new commit
//    Commit oldParent = oldCommit.parent(0);
//    git_repository *grepo = repo.data();

    // cherry-pick everything in the middle
    // cherry-pick old commit
    // cherry-pick everything after new commit
}

