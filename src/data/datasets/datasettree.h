/*
 * datasettree.h
 *
 *  Created on: 17.09.2013
 *      Author: Ralph
 */

#ifndef DATASETTREE_H_
#define DATASETTREE_H_

#include "dataset.h"

#include <QString>
#include <QVector>

class Tree;

class DatasetTree : public Dataset
{
public:
    DatasetTree( QDir fn = QDir( "hierarchical tree" ) );
    virtual ~DatasetTree();

    void draw( QMatrix4x4 pMatrix, QMatrix4x4 mvMatrix, int width, int height, int renderMode, QString target );

    void importTree( QString dims, QVector<QString>coords, QVector<QString>clusters );

    Tree* getTree() { return m_tree; };

private:
    Tree* m_tree;

    int m_numLeaves;
    int m_numNodes;
};

#endif /* DATASETTREE_H_ */
