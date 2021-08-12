import { HttpClient } from '@angular/common/http';
import { Component } from '@angular/core';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent {
  title = 'client';
  users: any;
  s:boolean = false;
  constructor(private http: HttpClient ){};

  GetUsers(){

    this.http.get("https://localhost:5001/api/users").subscribe(x => this.users = x);
    this.s = !this.s;
  }

}
