#include "EasyMotionTarget.h"

namespace EasyMotion{

EasyMotionTarget::EasyMotionTarget()
{
    m_targetPos.clear();
}

void EasyMotionTarget::clear()
{
    m_targetPos.clear();
}


int EasyMotionTarget::getFirstTargetIndex() const
{
    return 0;
}

int EasyMotionTarget::getLastTargetIndex() const
{
    return m_targetPos.size();
}

QPair<int, QString> EasyMotionTarget::getTarget(int i) const
{
    if (i < 0 || i > m_targetPos.size()) {
        return QPair<int, QString>(int(-1), QString::number(0));
    } else {
        return QPair<int, QString>(m_targetPos[i], QString::number(i+1));
    }
}


int EasyMotionTarget::getTargetPos(const int code) const
{
    int index = code - 1;
    if(index < 0 || index > m_targetPos.size())
        return -1;

    return m_targetPos[index];
}


} // namespace EasyMotion
