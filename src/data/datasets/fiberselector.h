/*
 * fiberselector.h
 *
 * Created on: Feb 26, 2013
 * @author schurade
 */

#ifndef FIBERSELECTOR_H_
#define FIBERSELECTOR_H_

#include "../../algos/fib.h"
#include "../../algos/kdtree.h"

#include <QVector>
#include <QObject>
#include <QAbstractItemModel>

class FiberSelector : public QObject
{
    Q_OBJECT

public:
    FiberSelector( int numPoints = 0, int numLines = 0 );
    FiberSelector( std::vector<float>* kdVerts, int numPoints = 0, int numLines = 0 );
    virtual ~FiberSelector();

    void init( std::vector<Fib>& fibs );

    std::vector<bool>* getSelection();
    QModelIndex createIndex( int branch, int pos, int column );

private:
    void updatePresentRois();

    int m_numLines;
    int m_numPoints;

    bool m_isInitialized;

    KdTree* m_kdTree;
    std::vector<float>* m_kdVerts;
    std::vector<int>m_reverseIndexes;
    std::vector<int>m_lineStarts;
    std::vector<int>m_lineLengths;

    std::vector<bool>m_rootfield;
    QList<std::vector<bool> >m_branchfields;
    QList<QList<std::vector<bool> > >m_bitfields;

    std::vector<float> m_boxMin;
    std::vector<float> m_boxMax;

    float m_x;
    float m_y;
    float m_z;
    float m_dx;
    float m_dy;
    float m_dz;

private slots:
    void roiChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );
    void roiInserted( const QModelIndex &parent, int start, int end );
    void roiDeleted( const QModelIndex &parent, int start, int end );

    void updateROI( int branch, int pos );
    void boxTest( std::vector<bool>& workfield, int left, int right, int axis );
    void sphereTest( std::vector<bool>& workfield );

    void updateBranch( int branch );
    void updateRoot();

signals:
    void changed();
};

#endif /* FIBERSELECTOR_H_ */
