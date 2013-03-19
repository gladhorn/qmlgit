
#include "gitrepo.h"


#include "gitdiff.h"

#include <git2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QObject>
#include <QString>
#include <QDir>
#include <QThread>

#include <QDebug>
#include <QQuickItem>

Git::Git()
    : m_gitCache(0), m_workerThread(0), m_repo(0), m_diffDirty(true), m_log(new GitCommitList())
{
    m_workerThread = new QThread(this);
    qDebug() << "main thread: " << thread()->currentThreadId();
}

Git::~Git()
{
    delete m_log;
    git_repository_free(m_repo);
}

void Git::setRepoUrl(const QString &url)
{
    m_url = url;
    git_repository_free(m_repo);
    m_repo = 0;
    m_commit.clear();
    m_branch.clear();
    if (m_gitCache) {
        m_workerThread->quit();
        m_gitCache->deleteLater();
    }
    m_gitCache = 0;

    m_log->setBranchData(QVector<git_commit*>());

    if (QDir::isAbsolutePath(url)) {
        // this probably needs unicode fixing on non-linux FIXME
        if (0 == git_repository_open_ext(&m_repo, url.toUtf8().data(), 0, NULL)) {
            m_diffDirty = true;
            m_gitCache = new GitCache(m_repo);
            connect(this, &Git::updateCache, m_gitCache, &GitCache::doWork);

            connect(m_gitCache, &GitCache::branchLoaded, this, &Git::branchLoaded);
            connect(m_gitCache, &GitCache::diffLoaded, this, &Git::diffLoaded);
            m_gitCache->moveToThread(m_workerThread);

            connect(m_gitCache, &GitCache::done, m_workerThread, &QThread::quit);
            connect(m_gitCache, &GitCache::statusChanged, this, &Git::setStatusMessage);
            m_workerThread->start();
        }
    }
    emit repoUrlChanged();
    emit diffChanged();
    emit modelChanged();
    emit branchesChanged();
}

bool Git::isValidRepository() const
{
    return m_repo != 0;
}

QStringList Git::branches()
{
    if (!m_repo)
        return QStringList();
    QStringList b;

    git_strarray ref_list;
    git_reference_list(&ref_list, m_repo, GIT_REF_LISTALL);

    for (uint i = 0; i < ref_list.count; ++i) {
        const char *refname;
        refname = ref_list.strings[i];
        git_reference *ref;
        git_reference_lookup(&ref, m_repo, refname);

        if (git_reference_is_branch(ref))
            b.append(refname);
    }
    git_strarray_free(&ref_list);

    return b;
}

QString Git::currentBranch() const
{
    return m_branch;
}

void Git::setCurrentBranch(const QString &branch)
{
    m_branch = branch;
    if (!branch.isEmpty()) {
        Q_ASSERT(m_gitCache);
        m_gitCache->loadBranch(branch);
        emit updateCache();
    }
}

void Git::branchLoaded(const QString &branch)
{
    if (branch == m_branch) {
        m_log->setBranchData(m_gitCache->branchData(branch));
        emit modelChanged();
    }
}

void Git::diffLoaded(const QString &commit)
{
    if (commit == m_commit)
        emit diffChanged();
}

QString Git::currentCommit() const
{
    return m_commit;
}

void Git::setCurrentCommit(const QString &commit)
{
    m_commit = commit;
    if (!m_commit.isEmpty()) {
        m_gitCache->loadDiff(commit);
        emit updateCache();
    }
    emit diffChanged();
}

QAbstractItemModel *Git::logModel()
{
    return m_log;
}

QString Git::diff() const
{
    if (!m_commit.isEmpty())
        return m_gitCache->diff(m_commit);
    return QString();
}

void Git::setStatusMessage(const QString &status)
{
    m_statusMessage = status;
    emit statusMessageChanged();
}

QML_DECLARE_TYPE(Git)
QML_DECLARE_TYPE(QAbstractItemModel)

