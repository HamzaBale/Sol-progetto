
using API.Entities;
using Microsoft.EntityFrameworkCore;

namespace API.Data
{
    public class DataContext : DbContext //è il ponte tra il database e le nostre entità, una sessione
    //di comunicazione in cui possiamo fare query o salvare oggetti.
    {
        public DataContext(DbContextOptions options) : base(options)
        {
        }
        public DbSet<AppUser> Users { get; set; } //DbSet è letteralmente una collezione di oggetti
        //cioè una tabella. In questo caso la tabella è formata da Users(Id,UserName) non penso numero
        //di telefono perchè è privato.
    }
}