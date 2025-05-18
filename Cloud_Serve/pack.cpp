#include "pack.h"

Pack::Pack():m_size(),m_type(),m_content(),m_binary_size(0)
{

}

QByteArray Pack::data()
{
    QJsonObject data_obj;
    data_obj["type"] = m_type;
    data_obj["content"] = QJsonArray::fromStringList(m_content);

    data_obj["binary_size"] = QString("%1").arg(m_binary_size, 4, 10, QChar('0'));


    QJsonDocument data_doc(data_obj);
    QByteArray data = data_doc.toJson();
    m_size = static_cast<uint>(data.length());
    QString formattedNumber = QString("%1").arg(m_size, 4, 10, QChar('0')); // 格式化为 4 位，不足补 0
    QJsonObject size_obj;
    //size_obj["size"] = static_cast<int>(m_size);
    size_obj["size"] = formattedNumber;
    QJsonDocument size_doc(size_obj);
    QByteArray size = size_doc.toJson();

    return (size + data);
}

void Pack::clear()
{
    m_content.clear();
}

uint Pack::get_header_len()
{
    QString formattedNumber = QString("%1").arg(m_size, 4, 10, QChar('0')); // 格式化为 4 位，不足补 0
    QJsonObject json_obj;
    //json_obj["size"] = static_cast<int>(m_size);
    json_obj["size"] = formattedNumber;

    QJsonDocument json_doc(json_obj);
    QByteArray json_data = json_doc.toJson();

    return static_cast<unsigned int>(json_data.length());
}

void Pack::set_content(const QString &data)
{
    m_content.append(data);
    return;
}

void Pack::set_binary_size(quint64 len)
{
    m_binary_size = len;
}

void Pack::set_type(const Type &type)
{
    m_type = type;
    return;
}
