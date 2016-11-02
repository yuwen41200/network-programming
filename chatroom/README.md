- [ ] Hello Message

      Send Format:
      > To the new connected client  
      > `[Server] Hello, anonymous! From: <Client IP>/<Client Port>`  
      > To existing clients  
      > `[Server] Someone is coming!`

- [ ] Offline Message

      Send Format:
      > To all the other clients  
      > `[Server] <USERNAME> is offline.`

- [ ] Who Message

      Recv Format:
      > `who`

      Send Format:
      > `[Server] <USERNAME> <CLIENT IP>/<CLIENT PORT>`  
      > `[Server] <SENDER USERNAME> <CLIENT IP>/<CLIENT PORT> ->me`

- [ ] Change Username Message

      Recv Format:
      > `name <NEW USERNAME>`

      Send Format:
      > If the new name is anonymous  
      > `[Server] ERROR: Username cannot be anonymous.`  
      > If the new name is not unique  
      > `[Server] ERROR: <NEW USERNAME> has been used by others.`  
      > If the new name does not consist of 2~12 English letters  
      > `[Server] ERROR: Username can only consists of 2~12 English letters.`  
      > To the user who changed his/her name  
      > `[Server] You're now known as <NEW USERNAME>.`  
      > To other users  
      > `[Server] <OLD USERNAME> is now known as <NEW USERNAME>.`

- [ ] Private Message

      Recv Format:
      > `tell <USERNAME> <MESSAGE>`

      Send Format:
      > If the sender's name is anonymous  
      > `[Server] ERROR: You are anonymous.`  
      > If the receiver's name is anonymous  
      > `[Server] ERROR: The client to which you sent is anonymous.`  
      > If the receiver doesn't exist  
      > `[Server] ERROR: The receiver doesn't exist.`  
      > To sender, if message is sent  
      > `[Server] SUCCESS: Your message has been sent.`  
      > To receiver, if both client's name are not anonymous  
      > `[Server] <SENDER USERNAME> tell you <MESSAGE>`

- [ ] Broadcast Message

      Recv Format:
      > `yell <MESSAGE>`

      Send Format:
      > `[Server] <SENDER USERNAME> yell <MESSAGE>`

- [ ] Error Command

      Send Format:
      > `[Server] ERROR: Error command.`
