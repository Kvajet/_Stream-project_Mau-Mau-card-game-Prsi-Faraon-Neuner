#include "CAce.h"
#include "CJoker.h"
#include "CSeven.h"
#include "CBasicCard.h"
#include "CGameRegister.h"
#include <memory>
#include <vector>
#include <ctime>

class CDeck
{
public:
    CDeck( const std::shared_ptr< CGameRegister > & gameRegister , std::vector< std::shared_ptr< CCard > > & hand1 , std::vector< std::shared_ptr< CCard > > & hand2 )
        : m_register( gameRegister ) , m_hand1( hand1 ) , m_hand2( hand2 )
    {
        m_pile.reserve( 32 );
        m_randBuffer.reserve( 32 );

        std::srand( std::time( nullptr ) );

        Generate();
        Randomize();
        // Test();

        for( size_t i = 0 ; i < 4 ; i++ )
        {
            m_hand1.push_back( DrawOne() );
            m_hand2.push_back( DrawOne() );
        }
        
        m_register->AssignCard( m_pile.back() );
        m_usedCardsVec.push_back( m_pile.back() );
        m_pile.erase( m_pile.begin() + 32 - m_usedCards );

        // std::cout << "Total cards " << ( m_usedCardsVec.size() + m_pile.size() + m_hand1.size() + m_hand2.size() ) << "\n"; 
    }

    void Draw()
    {
        if( m_pile.size() )
        {
            if( m_register->player ) m_hand2.push_back( DrawOne() );
            else                     m_hand1.push_back( DrawOne() );
        }
    }

    void ShowTop() const;

    size_t RemainingCardsInPile() const
    {
        return m_pile.size();
    }

private:
    void Test() const
    {
        for( const auto & it : m_pile )
            it->Action();
    }

    void Generate()
    {
        for( char i = CCard::Color::ACORNS ; i <= CCard::Color::BELLS ; i++ )
        {
            m_pile.push_back( std::make_shared< CAce >( static_cast< CCard::Color >( i ) ) );
            m_pile.push_back( std::make_shared< CJoker >( static_cast< CCard::Color >( i ) ) );
            m_pile.push_back( std::make_shared< CSeven >( static_cast< CCard::Color >( i ) ) );
            for( char k = CBasicCard::BasicType::UNTER ; k <= CBasicCard::BasicType::KING ; k++ )
                m_pile.push_back( std::make_shared< CBasicCard >( static_cast< CCard::Color >( i ) , static_cast< CBasicCard::BasicType >( k ) ) );
        }
    }

    std::shared_ptr< CCard > DrawOne()
    {
        unsigned int remainingCards = 32 - m_usedCards;

        std::shared_ptr< CCard > tmp = m_pile.back();
        m_pile.erase( m_pile.begin() + remainingCards );
        m_usedCards++;

        return tmp;
    }

    void Randomize()
    {
        unsigned int remainingCards = 32 - m_usedCards;

        m_randBuffer = m_pile;
        m_pile.clear();

        int rand;
        while( remainingCards )
        {
            rand = std::rand() % remainingCards;
            auto it = m_randBuffer.begin() + rand;
            m_pile.push_back( *it );
            m_randBuffer.erase( it );
            remainingCards--;
        }
    }

    size_t m_usedCards = 0;
    std::vector< std::shared_ptr< CCard > > m_pile;
    std::vector< std::shared_ptr< CCard > > m_randBuffer;
    std::vector< std::shared_ptr< CCard > > m_usedCardsVec;

    std::shared_ptr< CGameRegister > m_register;
    std::vector< std::shared_ptr< CCard > > & m_hand1;
    std::vector< std::shared_ptr< CCard > > & m_hand2;
};