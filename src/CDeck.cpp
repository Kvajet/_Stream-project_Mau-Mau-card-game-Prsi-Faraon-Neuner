#include "CDeck.h"

CDeck::CDeck( const std::shared_ptr< CGameRegister > & gameRegister ,
              std::vector< std::shared_ptr< CCard > > & hand1 ,
              std::vector< std::shared_ptr< CCard > > & hand2 ,
              CGameRegister::PlayMode & mode ,
              const size_t & colorIndex )
    : m_register( gameRegister ) , 
      m_hand1( hand1 ) , 
      m_hand2( hand2 ) ,
      m_mode( mode ) , 
      m_colorIndex( colorIndex )
{
    Reserve();
    std::srand( std::time( nullptr ) );

    Generate();
    Randomize();

    for( size_t i = 0 ; i < START_CARDS * 2 ; i++ )
    {
        DrawOne();
        m_register->ChangePlayer();
    }
    
    m_register->AssignCard( m_pile.back() , m_pile , true );
    m_usedCardsVec.push_back( m_pile.back() );
    m_pile.pop_back();

    if( m_register->m_lastCard->Type() == CCard::BasicType::JOKER ) m_register->m_actColor = m_pile.front()->CColor();
}

void CDeck::Draw( bool innerCall )
{
    CCard::BasicType type = m_register->m_lastCard->Type();
    
    if( ! m_register->m_lastResolved && ( type == CCard::BasicType::SEVEN || type == CCard::BasicType::ACE ) )
    {
        if( m_register->m_lastCard->Type() == CCard::BasicType::SEVEN )
        {
            ActionSeven();
            m_register->m_lastResolved = true;
        }
        return;
    }

    DrawOne();
    if( ! innerCall ) m_register->ChangePlayer();
}

void CDeck::Play()
{
    if( m_mode == CGameRegister::PlayMode::JOKER_MODE )
    {
        m_register->m_actColor = static_cast< CCard::Color>( m_colorIndex );
        m_mode = CGameRegister::PlayMode::NORMAL_MODE;
        return;
    }

    std::vector< std::shared_ptr < CCard > > & hand = m_register->player ? m_hand2 : m_hand1; 
    size_t index = m_register->player ? m_register->m_player2handIndex : m_register->m_player1handIndex;

    if( ! m_register->m_lastResolved )
    {
        switch( m_register->m_lastCard->Type() )
        {
            case CCard::BasicType::SEVEN:
                if( hand[ index ]->Type() == CCard::BasicType::SEVEN )
                {
                    m_register->m_sevenAmplifier++;
                    DropCard( hand , index );
                }
                else
                { 
                    Draw();
                    m_register->m_lastResolved = true;
                }
                return;
            case CCard::BasicType::ACE:
                if( hand[ index ]->Type() == CCard::BasicType::ACE ) DropCard( hand , index );
                else                                                 ActionAce();
                return;
            case CCard::BasicType::JOKER: 
                if( hand[ index ]->CColor() == m_register->m_actColor ) ActionJoker( hand , index );
                return;
            default:
                break;
        }
    }
    else
    {
        if( hand[ index ]->Type() == CCard::BasicType::JOKER )
        {
            m_mode = CGameRegister::PlayMode::JOKER_MODE;
            DropCard( hand , index , true );
            m_register->m_lastResolved = false;
            return;
        }
        DropCard( hand , index );
    }
}

size_t CDeck::RemainingCardsInPile() const
{
    return m_pile.size();
}

const std::vector< std::shared_ptr< CCard > > & CDeck::GetPile() const
{
    return m_pile;
}

void CDeck::Generate()
{
    for( char i = CCard::Color::ACORNS ; i <= CCard::Color::BELLS ; i++ )
        for( char k = CCard::BasicType::UNTER ; k <= CCard::BasicType::SEVEN ; k++ )
            m_pile.push_back( std::make_shared< CCard >( static_cast< CCard::Color >( i ) , static_cast< CCard::BasicType >( k ) ) );
}

void CDeck::DropCard( std::vector< std::shared_ptr < CCard > > & hand , size_t index , bool innerCall )
{
    std::shared_ptr< CCard > currentCard = hand[ index ];

    if( currentCard == m_register->m_lastCard || innerCall )
    {
        m_register->AssignCard( currentCard , m_pile );
        hand.erase( hand.begin() + index );
        m_usedCardsVec.push_back( currentCard );
        if( index >= hand.size() ) m_register->player ? m_register->m_player2handIndex-- : m_register->m_player1handIndex--;
        m_register->ChangePlayer();
    }
}

void CDeck::DrawOne()
{
    if( ! m_pile.size() && m_usedCardsVec.size() > 1 )
    {
        std::shared_ptr< CCard > lastCardHolder = m_usedCardsVec.back();
        m_usedCardsVec.pop_back();

        while( m_usedCardsVec.size() )
        {
            m_pile.push_back( m_usedCardsVec.back() );
            m_usedCardsVec.pop_back();
        }
        
        m_usedCardsVec.push_back( lastCardHolder );
    }

    std::shared_ptr< CCard > tmp = m_pile.back();
    m_pile.pop_back();
    m_usedCards++;
    
    if( m_pile.size() )
    {
        if( m_register->player ) m_hand2.push_back( tmp );
        else                     m_hand1.push_back( tmp );
    }
}

void CDeck::Randomize()
{
    unsigned int remainingCards = TOTAL_CARDS - m_usedCards;

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

bool CDeck::HasCounterplay() const
{
    const std::vector< std::shared_ptr< CCard > > & hand = m_register->player ? m_hand2 : m_hand1;

    switch( m_register->m_lastCard->Type() )
    {
        case CCard::BasicType::SEVEN:
            for( const auto & it : hand )
                if( it->Type() == CCard::BasicType::SEVEN ) return true;
            break;
        case CCard::BasicType::ACE:
            for( const auto & it : hand )
                if( it->Type() == CCard::BasicType::ACE ) return true;
            break;
        case CCard::BasicType::JOKER: break;
        default:                      break;
    }

    return false;
}

void CDeck::ActionSeven()
{
    for( size_t i = 0 ; i < 2 * m_register->m_sevenAmplifier ; i++ ) DrawOne();
    m_register->ChangePlayer();
    m_register->m_lastResolved = true;
    m_register->m_sevenAmplifier = 1;
}

void CDeck::ActionJoker( std::vector< std::shared_ptr < CCard > > & hand , size_t index )
{
    DropCard( hand , index , true );
    m_register->m_lastResolved = true;
}

void CDeck::ActionAce()
{
    m_register->ChangePlayer();
    m_register->m_lastResolved = true;
}

void CDeck::Reserve()
{
    m_pile.reserve( TOTAL_CARDS );
    m_randBuffer.reserve( TOTAL_CARDS );
    m_usedCardsVec.reserve( TOTAL_CARDS );
    m_hand1.reserve( TOTAL_CARDS );
    m_hand2.reserve( TOTAL_CARDS );
}