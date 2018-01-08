#include <iostream>
#include <stdlib.h>

#include <cassert>
#include <string.h>
#include <vector>


class hand
{
public:
    enum suit_t { FIRST_SUIT=0, CLUB=0, DIAMOND, HEART, SPADE, SUIT_COUNT };
    enum card_t { FIRST_CARD=0, ACE=0, KING, QUEEN, JACK, TEN, NINE, EIGHT, SEVEN, SIX, FIVE, FOUR, THREE, TWO, CARD_COUNT };
    enum card_mask_t
    {
        ACE_MASK = 1<<ACE,
        KING_MASK = 1<<KING,
        QUEEN_MASK = 1<<QUEEN,
        JACK_MASK = 1<<JACK,
        TEN_MASK = 1<<TEN,
        NINE_MASK = 1<<NINE,
        EIGHT_MASK = 1<<EIGHT,
        SEVEN_MASK = 1<<SEVEN,
        SIX_MASK = 1<<SIX,
        FIVE_MASK = 1<<FIVE,
        FOUR_MASK = 1<<FOUR,
        THREE_MASK = 1<<THREE,
        TWO_MASK = 1<<TWO
    };

private:
    static int bit_pos( u_int64_t n ) { return __builtin_ctzl(n); }
    static int bit_count( u_int64_t n ) { return __builtin_popcountl(n); }

    u_int64_t value_ = 0;

    static int suit_offset( suit_t s ) { return s*16; }

    /// A 4 bits suit mask
    static int suit_mask( suit_t s) { return 1<<s; }

    /// A 13 bits card mask
    static u_int64_t card_mask( card_t c ) { return 1<<c; }

    /// A 13 (really 16) '1' bits representing where the cards of this suit are
    static u_int64_t suit_card_mask( suit_t s ) { return 0xfffful<<suit_offset(s); }

    /// The first bit for the card in the suit
    static u_int64_t suit_first_card_mask( suit_t s ) { return 1ul<<suit_offset(s); }

    int cards_in_suit( suit_t s ) const { return (value_>>suit_offset(s))&0xffff; }

    static const char *suit_names() { return "CDHS"; }
    static const char *card_names() { return "AKQJT98765432"; }

    static char name_from_suit( suit_t s ) { return suit_names()[s]; }
    static char name_from_card( card_t s ) { return card_names()[s]; }

    static suit_t suit_from_name( char n ) { return (suit_t)(strchr(suit_names(),n)-suit_names()); }
    static card_t card_from_name( char n ) { return (card_t)(strchr(card_names(),n)-card_names()); }

    /// Number of cards in the hand
    int card_count() const { return bit_count(value_); }

    /// Return a mask of all suits present
    int suits() const
    {
        return
            (cards_in_suit(DIAMOND)!=0)*suit_mask(DIAMOND)
        +   (cards_in_suit(HEART)!=0)*suit_mask(HEART)
        +   (cards_in_suit(CLUB)!=0)*suit_mask(CLUB)
        +   (cards_in_suit(SPADE)!=0)*suit_mask(SPADE);
    }

    /// Return a mask of all cards present
    int cards() const
    {
        return cards_in_suit(DIAMOND)|cards_in_suit(HEART)|cards_in_suit(CLUB)|cards_in_suit(SPADE);
    }

    explicit hand( u_int64_t v ) : value_{v} {}


public:
    hand() {}

    hand( card_t c, suit_t s ) :
        value_{ card_mask(c)<<suit_offset(s) }
    {
    }

    hand( char c, char s )
    {
        *this = hand{ card_from_name(c), suit_from_name(s) };
    }

    hand( const char *n )
    {
        std::vector<hand> v;
        while (*n)
        {    v.push_back( hand{ n[0], n[1] } );
             n += 2;
             if (*n=='/')
                n++;
        }
        *this = hand{ v };
    }

    hand( std::initializer_list<hand> l )
    {
        for (auto h:l)
            value_ |= h.value_;
    }

    hand( const std::vector<hand> &v )
    {
        for (auto h:v)
            value_ |= h.value_;
    }

    std::string name_single() const
    {
        char buffer[3] =
        {
            name_from_card((card_t)bit_pos(cards())),
            name_from_suit((suit_t)bit_pos(suits())),
            0
        };

        return buffer;
    }

    std::string name() const
    {
        std::string result = "";
        std::string sep = "";
        for (u_int64_t i=0;i!=64;i++)
            if (value_&(1ul<<i))
            {
                result += sep+hand{1ul<<i}.name_single();
                sep = "/";
            }
        return result;
    }

    void operator-=( hand h )
    {
        value_ &= ~h.value_;
    }

    static bool check_mask( int v, int m ) { return (v&m)==m; }

    /// Return a flush value: 0: no flush, 1: ace flush, to 10: five flush
    static int flush_from_card_mask( int m )
    {
        if (bit_count(m)<5)
            return 0;
        if (check_mask( m, ACE_MASK|KING_MASK|QUEEN_MASK|JACK_MASK|TEN_MASK ))
            return ACE+1;
        if (check_mask( m, KING_MASK|QUEEN_MASK|JACK_MASK|TEN_MASK|NINE_MASK ))
            return KING+1;
        if (check_mask( m, QUEEN_MASK|JACK_MASK|TEN_MASK|NINE_MASK|EIGHT_MASK ))
            return QUEEN+1;
        if (check_mask( m, JACK_MASK|TEN_MASK|NINE_MASK|EIGHT_MASK|SEVEN_MASK ))
            return JACK+1;
        if (check_mask( m, TEN_MASK|NINE_MASK|EIGHT_MASK|SEVEN_MASK|SIX_MASK ))
            return TEN+1;
        if (check_mask( m, NINE_MASK|EIGHT_MASK|SEVEN_MASK|SIX_MASK|FIVE_MASK ))
            return NINE+1;
        if (check_mask( m, EIGHT_MASK|SEVEN_MASK|SIX_MASK|FIVE_MASK|FOUR_MASK ))
            return EIGHT+1;
        if (check_mask( m, SEVEN_MASK|SIX_MASK|FIVE_MASK|FOUR_MASK|THREE_MASK ))
            return SEVEN+1;
        if (check_mask( m, SIX_MASK|FIVE_MASK|FOUR_MASK|THREE_MASK|TWO_MASK ))
            return SIX+1;
        if (check_mask( m, FIVE_MASK|FOUR_MASK|THREE_MASK|TWO_MASK|ACE_MASK ))
            return FIVE+1;
        return 0;
    }

#define BASE_SF 1   //  Straight Flush

    /// If non-zero return, straight flush value
    int value_sf()
    {
        auto v = flush_from_card_mask( cards_in_suit(DIAMOND) );
        if (!v) v = flush_from_card_mask( cards_in_suit(HEART) );
        if (!v) v = flush_from_card_mask( cards_in_suit(CLUB) );
        if (!v) v = flush_from_card_mask( cards_in_suit(SPADE) );
        if (v) v = v-1+BASE_SF;
        return v;
    }

#define BASE_FK 11    //  Four of a Kind

    int value_fk()
    {
        u_int64_t four = cards_in_suit(DIAMOND)&cards_in_suit(HEART)&cards_in_suit(CLUB)&cards_in_suit(SPADE);
        if (four)
        {
            hand h{ *this };
            h -= hand{ four };  //  #### Moche et lent
            h -= hand{ four<<16 };
            h -= hand{ four<<32 };
            h -= hand{ four<<48 };
            int found_card = bit_pos(four);
            int highest_missing = bit_pos(h.cards());
            return BASE_FK + found_card*12 + highest_missing - (highest_missing>found_card);
        }
    }

#define BASE_FH 167     //  Full House

    static int value_fh32( int *counts )
    {
        /// Look for 3, then 2 in the array
        for (int c3=FIRST_CARD;c3!=CARD_COUNT;c3++)
        {
            if (*counts==3)
            {
                for (int c2=c3+1;c2!=CARD_COUNT;c2++)
                {
                    counts++;
                    if (*counts>=2)
                        return BASE_FH+12*c3+c2-1;
                }
                return 0;
            }
            counts++;
        }
        return 0;
    }

    static int value_fh23( int *counts )
    {
        /// Look for 2, then 3 in the array
        for (int c2=FIRST_CARD;c2!=CARD_COUNT;c2++)
        {
            if (*counts==2)
            {
                for (int c3=c2+1;c3!=CARD_COUNT;c3++)
                {
                    counts++;
                    if (*counts==3)
                        return BASE_FH+12*c3+c2;
                }
                return 0;
            }
            counts++;
        }
        return 0;
    }

    int value_fh()
    {
        int cards_count[CARD_COUNT]{};
        u_int64_t mask = suit_first_card_mask(DIAMOND)|suit_first_card_mask(HEART)|suit_first_card_mask(CLUB)|suit_first_card_mask(SPADE);
        bool found2_first = false;
        bool found2 = false;
        bool found3 = false;

        for (int c=FIRST_CARD;c!=CARD_COUNT;c++)
        {
            auto count = bit_count( value_&mask );
            found2 |= (count==2);
            if (found2 && !found3 && !found2_first)
                found2_first = true;
            found3 |= (count==3);
            cards_count[c] = count;
            mask <<= 1;
        }

        if (found3)
        {
            if (found2_first)
                return value_fh23( cards_count );
            else
                return value_fh32( cards_count );
        }

        return 0;
    }

#define BASE_F

    int value_f()
    {
        return 42;
    }

    static void tests()
    {
            //  Card flush tests
        assert( flush_from_card_mask( hand{"AC/KC/QC/JC/TC"}.cards() )==1 );
        assert( flush_from_card_mask( hand{"KC/QC/JC/TC/9C"}.cards() )==2 );
        assert( flush_from_card_mask( hand{"5C/4C/3C/2C/AC"}.cards() )==10 );
        assert( !flush_from_card_mask( hand{"6C/4C/3C/2C/AC"}.cards() ) );

            //  Straight flush tests
        assert( hand{"AC/KC/QC/JC/TC"}.value_sf()==BASE_SF );
        assert( !hand{"AS/KC/QC/JC/TC"}.value_sf() );
        assert( hand{"AC/KC/QC/JC/TC/2D/3H"}.value_sf()==BASE_SF );
        assert( hand{"KC/QC/JC/TC/2D/3H/9C"}.value_sf()==BASE_SF+1 );

            //  Four of a kind tests
        std::cout << hand{"AC/AD/AH/AS/KC"}.value_fk();
        assert( hand{"AC/AD/AH/AS/KC"}.value_fk()==BASE_FK );
        assert( hand{"AC/AD/AH/AS/QC"}.value_fk()==BASE_FK+1 );
        assert( hand{"AC/AD/AH/AS/2C"}.value_fk()==BASE_FK+11 );
        assert( hand{"AC/AD/AH/AS/2C/KC"}.value_fk()==BASE_FK );
        assert( hand{"2C/2D/2H/2S/3C"}.value_fk()==BASE_FK+12*13-1 );   //  Lowest FK

            //  Full
        assert( !hand{ "AC/AD/AH" }.value_fh() );
        assert( hand{ "AC/AD/AH/KC/KD" }.value_fh()==BASE_FH );
        assert( hand{ "KC/KD/KH/AC/AD" }.value_fh()==BASE_FH+12 );
    }

    int best_value() const
    {
        assert( bit_count( value_ )>=5 );

        //  We check the four colors for flush

        return 0;
    }
};

int main( void )
{
    hand h1 { hand::ACE, hand::SPADE };
    assert( h1.name()=="AS" );

    hand h2 { hand{ hand::TWO, hand::SPADE }, hand{ hand::ACE, hand::HEART } };
    assert( h2.name()=="AH/2S" );

    hand h3 { hand{ hand::ACE, hand::DIAMOND }, hand{ hand::ACE, hand::HEART }, hand{ hand::ACE, hand::CLUB }, hand{ hand::ACE, hand::SPADE } };
    assert( h3.name()=="AC/AD/AH/AS" );

    hand h4 { hand{ hand::TWO, hand::CLUB }, hand{ hand::THREE, hand::CLUB }, hand{ hand::FOUR, hand::CLUB }, hand{ hand::FIVE, hand::CLUB }, hand{ hand::SIX, hand::CLUB }, hand{ hand::SEVEN, hand::CLUB }, hand{ hand::EIGHT, hand::CLUB }, hand{ hand::NINE, hand::CLUB }, hand{ hand::TEN, hand::CLUB }, hand{ hand::JACK, hand::CLUB }, hand{ hand::QUEEN, hand::CLUB }, hand{ hand::KING, hand::CLUB }, hand{ hand::ACE, hand::CLUB } };

    assert( h4.name()=="AC/KC/QC/JC/TC/9C/8C/7C/6C/5C/4C/3C/2C" );

    assert( hand{ "AC"} .name()=="AC" );
    assert( hand{ "AC/AD/AH/AS"} .name()=="AC/AD/AH/AS" );
    assert( hand{ "ACADAHAS"} .name()=="AC/AD/AH/AS" );
    assert( hand{ "ADACASAH"} .name()=="AC/AD/AH/AS" );

    std::cout << hand{ "ACADASAH2S" }.best_value() << std::endl;

    hand::tests();

    return EXIT_SUCCESS;
}
