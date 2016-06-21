#include "treenode.h"
#include "token.h"

TreeNode::TreeNode(int type)
{
    m_left = NULL;
    m_middle = NULL;
    m_right = NULL;
    m_next = NULL;
    m_type = type;
    m_lineNo = 0;
    m_dataType = -1;
    switch (m_type)
    {
    case FACTOR:
        m_dataType = Token::PLUS;
        break;
    }
}

TreeNode::~TreeNode()
{
    if (m_left != NULL)
        delete m_left;
    if (m_middle != NULL)
        delete m_middle;
    if (m_right != NULL)
        delete m_right;
    if (m_next != NULL)
        delete m_right;
}

TreeNode * TreeNode::left() const
{
    return m_left;
}

void TreeNode::setLeft(TreeNode *left)
{
    if (m_left != NULL)
        delete m_left;
    m_left = left;
}
TreeNode * TreeNode::middle() const
{
    return m_middle;
}

void TreeNode::setMiddle(TreeNode *middle)
{
    if (m_middle != NULL)
        delete m_middle;
    m_middle = middle;
}
TreeNode * TreeNode::right() const
{
    return m_right;
}

void TreeNode::setRight(TreeNode *right)
{
    if (m_right != NULL)
        delete m_right;
    m_right = right;
}
TreeNode * TreeNode::next() const
{
    return m_next;
}

void TreeNode::setNext(TreeNode *next)
{
    if (m_next != NULL)
        delete m_right;
    m_next = next;
}
int TreeNode::type() const
{
    return m_type;
}

void TreeNode::setType(int type)
{
    m_type = type;
}
int TreeNode::dataType() const
{
    return m_dataType;
}

void TreeNode::setDataType(int dataType)
{
    m_dataType = dataType;
}
std::string TreeNode::value() const
{
    return m_value;
}

void TreeNode::setValue(const std::string &value)
{
    m_value = value;
}
int TreeNode::lineNo() const
{
    return m_lineNo;
}

void TreeNode::setLineNo(int lineNo)
{
    m_lineNo = lineNo;
}









