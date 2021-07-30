#include <iostream>

template<typename Item>
class LinkedList
{
    private:
        // Структура которая реализует узел списка
        struct Node
        {
            Item item; // Содержимое узла
            Node *prev; // Указатель на предыдущий узел
            Node *next; // Указатель на следующий узел
        };

        Node *first; // Указатель на первый узел списка
        Node *last; // Указатель на последний узел списка
        int size; // Размер списка

        // Перечисление используемое в приватном методе ниже
        enum{
            up = 0,
            down = 1
        };

        // Метод, исключающий дублирование кода при поиске элементов внутри класса
        Node& findElement(int pos, int count = up) const;
    
    public:
        // Конструктор по умолчанию
        LinkedList();
        // Конструктор с созданием одного узла
        LinkedList(Item);
        // Конструктор копирования (глубокое копривание)
        LinkedList(const LinkedList&);

        // Деструктор
        ~LinkedList();

        // Добавление узла в конец списка
        LinkedList& Add(const Item&);
        // Добавление узла на указаную позицию
        LinkedList& Add(const Item&, int pos);
        // Размещение указанного количества элементов массива в конце списка
        LinkedList& Add(const Item[], int number);
        //То же что и выше только с указанием позиции размещения
        LinkedList& Add(const Item[], int number, int pos);
        // Добавление узлов из другого списка
        LinkedList& Add(const LinkedList<Item>&);
        // То же только с добавлением на указанную позицию
        LinkedList& Add(const LinkedList<Item>&, int pos);

        // Удаление узлов начиная с узла который указан в pos
        // (по умолчанию удаляется один узел)
        LinkedList& Delete(int pos, int range = 1);
        // Удаление узла в конце списка
        LinkedList& Delete();

        // Перегрузка операции [] используется при использовании
        Item operator[](int i) const;
        // То же только используется в изменяемом списке
        Item& operator[](int i);

        // перешрузка оператора сложения
        LinkedList operator+(const LinkedList<Item>&) const;
        // Перешрузка операции равно (глубокое копирование)
        const LinkedList& operator=(const LinkedList<Item>&);

        // Метод возвращает размер списка
        int Size();
};

/************************************************************************/

// Первый метод, позволяющий найти нужный узел в списке
// Без ипользования typename, возникает проблема распознования 
// Включенной структуры Node, как типа, поэтому необходимо это явно указать
template<typename Item>
typename LinkedList<Item>::Node& LinkedList<Item>::findElement(int pos, int count) const
{
    LinkedList<Item>::Node* current; // Указатель на узел

    // Теперь в зависимости от значения count будет осуществляться поиск 
    // либо сверху вниз, либо снизу вверх
    // (при удалении элементов может быть невозможен поиск снизу вверх)
    // При удалении например из центра списка
    // при прохождении его от первого элемента наталкиваемся на пустую область памяти
    // В этом случае придется идти от последнего элемента к нужному нам
    if(count == up)
    {
        current = first;
        for(int k = 1; k <= pos; k++)
            current = current->next;
    }
    else
    {
        current = last;
        for(int k = size - 2; k >= pos; k--)
            current = current->prev;
    }

    return *current;
    
}

// Основной метод, добавляющий узел (newNode)
// Будет использоваться в остальных перегрузках метода Add
template<typename Item>
LinkedList<Item>& LinkedList<Item>::Add(const Item& item, int pos)
{
    // Создаем новый узел и инициализируем им указатель
    LinkedList<Item>::Node* temp = new LinkedList<Item>::Node;
    // Устанавиваем содержимое нового узла
    temp->item = item;

    // Если позиция указывает за последним элементом, то 
    if(pos >= size)
    {
        // новый узел становится последним и
        // указатель на следующий элемент узла устанавливается в "0"
        last = temp;
        temp->next = nullptr;
    }
    else // В ином случае
    {
        // Указатель на следующий элемент устанавливается на узел
        // который ранее стоял на этой позиции (oldNode)
        temp->next = &findElement(pos);
        // а указатель на предыдущий узел узла oldNode устанавливается на newNode
        (temp->next)->prev = temp;
    }
    // В итоге новый узел и узел перед ним(если он есть) связаны,
    // Осталось связать наш узел с предыдущим

    // Т.к либо перед, либл после, либо же по обе стороны от нашего узла
    // могут отсутствовать другие узлы
    // то логично было бы разделить связвание newNode c последующим узлом
    // Это избавляет нас от сильной вложенности и путаницы
    

    // Если позиция = 0 то узел становится первым
    if(pos == 0)
    {
        temp->prev = nullptr;
        first = temp;
    }
    else
    {
        // В ином случае, если размер не равен нулю
        // (вдруг пользователь указал не нулевую позицию а список пуст)
        // то связвание newNode c предыдущим узлом
        if(size != 0)
        {
            temp->prev = &findElement(pos-1);
            temp->prev->next = temp;
        }
    }
    size++; // Увеличиваем значение размера списка
    // Возвращаем используя ссылку объект который вызвал метод
    // чтобы можно было воспользоваться конструкцией вида: list.Add().Add()
    return *this;
}

// Добавление узла в конец списка
template<typename Item>
LinkedList<Item>& LinkedList<Item>::Add(const Item& item)
{
    return Add(item, size);
}

// Добавление массива элементов item[] в список,
// где number - количество элементов массива,
// которые необходимо добавить а pos указывает позицию 
// с которой начинается добавление узлов
template<typename Item>
LinkedList<Item>& LinkedList<Item>::Add(const Item item[], int number, int pos)
{
    for(int i = 0; i < number; i++, pos++)
    {
        Add(item[i], pos);
    }
    return *this;
}

// Добавление массива в конец списка
template<typename Item>
LinkedList<Item>& LinkedList<Item>::Add(const Item item[], int number)
{
    return Add(item, number, size);
}

// Добавление в список другого списка
// Узлы добовляются начиная с указаной позиции
template<typename Item>
LinkedList<Item>& LinkedList<Item>::Add(const LinkedList<Item>& list, int pos)
{
    for(int i = 0; i < list.size; i++, pos++)
    {
        Add(list[i], pos);
    }

    return *this;
}


// Добавление  в конец списка другого списка
template<typename Item>
LinkedList<Item>& LinkedList<Item>::Add(const LinkedList<Item>& list)
{
    return Add(list, size);
}


template<typename Item>
LinkedList<Item>::LinkedList()
{
    size = 0;
    first = nullptr;
    last = nullptr;
}

template<typename Item>
LinkedList<Item>::LinkedList(Item item) // Создание одного узла
{
    size = 1;
    // Создание узла и установка на него указателей
    first = last = new LinkedList<Item>::Node;
    // Через указатель first устанавливаем поля созданого узла
    first->prev = nullptr;
    first->next = nullptr;
    first->item = item;
}

// Конструктор, инициализирующий создание объекта списком
template<typename Item>
LinkedList<Item>::LinkedList(const LinkedList<Item>& ll)
{
    first = last = nullptr;
    size = 0;
    for(int i = 0; i < ll.size; i++)
    {
        Add(ll[i], i);
    }
}

template<typename Item>
Item& LinkedList<Item>::operator[](int i)
{
    if(i < 0 || i >= size)
        std::cout << "Выход за границы массива";
    // С помощью приватного метода ихется нужный элемент
    // и возвращается его полезное содержимое 
    return findElement(i).item;
}

// То же самое но для работы с константными данными
template<typename Item>
Item LinkedList<Item>::operator[](int i) const
{
    if(i < 0 || i >= size)
        std::cout << "Выход за границы массива";
    
    return findElement(i).item;
}

// По умолчанию удаляется один элемент на коорый указывает pos
template<typename Item>
LinkedList<Item>& LinkedList<Item>::Delete(int pos, int range)
{
    // Костыль
    if(pos < 0 || pos >= size)
        std::cout << "Выход за границы массива";
    
    // Так как в методе findElement используется после size
    // то до полного сшивания списка размер не должен изменяться
    // в ином случае поиск будет осуществляться неверно
    // Поэтому в самом методе используется tempSize
    int tempSize = size;

    // Первый этап
    // Последовательно удаляем нужное количество элементов 
    // начиная с указаной позиции
    for(int i = pos; i < pos + range && i < size; i++, tempSize--)
        delete &findElement(i, down);

    // Второй этап
    // Если были удалены все элементы 
    // то просто устанавливаем все указатели в ноль
    if(tempSize == 0)
        first = last = nullptr;
    else
    {
        // В ином случае возможны три варианта:
        // 1) Удалено определенное количество элементов, начиная с первого
        // 2) Удалены узлы (не м первого) вплоть до последнего включительно
        // 3) Удалены узлы (не с первого) но не доходя до последнего узла
        if(pos == 0) // 1)
        {
            // Обновляем первый элемент списка
            first = &findElement(pos + range, down);
            first->prev = nullptr;
        }
        else if(pos + range >= size) // 2
        {
            // Обновляем последний элемент списка
            last = &findElement(pos - 1);
            last->next = nullptr;
        }
        else // 3
        {
            // Сшиваем конец одного куска списка с началом другого 
            // down указывет нам о том, что поиск элемента идет сверху вниз
            // Логично, что в одних случаях надо идти свеху вниз
            // а в других - наоборот
            // Это вызвано появлением "пустого" участка в центре списка, ибо два куска его
            // еще не сшиты между собой. Но у нас имеются указатели last и first,
            // каждый из которых указывает на отдельный кусок и можно таким образом 
            // проводить поиск в каждом из них
            findElement(pos - 1).next = &findElement(pos + range, down);
            findElement(pos + range, down).prev = &findElement(pos - 1);
        }
    }
      // Обновляем значение размера списка
        size = tempSize;
        return *this;
}

// Удаление элемента в конце списка
template<typename Item>
LinkedList<Item>& LinkedList<Item>::Delete()
{
    return Delete(size - 1);
}

template<typename Item>
LinkedList<Item>::~LinkedList()
{
    Delete(0, size);
}

template<typename Item>
LinkedList<Item> LinkedList<Item>::operator+(const LinkedList<Item>& list) const
{
    // Создание временный объект, инициализируется
    // вызвавшим его объектом
    LinkedList<Item> temp(*this);
    // Добавляем второй список
    temp.Add(list);
    return temp;
}

template<typename Item>
const LinkedList<Item>& LinkedList<Item>::operator=(const LinkedList<Item>& list)
{
    // Очистка списка
    if(size != 0)
        this->~LinkedList();
    // Запись в список новых узлов
    Add(list);

    // Можно использовать так list1 = list2 = list3
    return list;
}

template<typename Item>
int LinkedList<Item>::Size()
{
    return size;
}


int main()
{
   
   int item[5] = {45,56,34,23,78};

    LinkedList<int> list;
    list.Add(item, 5, 0);
    std::cout << list.Size() << std::endl;
    return 0;
}